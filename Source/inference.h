#pragma once
#include <JuceHeader.h>

#include <torch/torch.h>
#include <torch/script.h>

// #define LOAD_FROM_PATH = "Source/Data/PRISM_traced_model_8bands.pth" // Only for debugging locally, load model pth from file path

class InferenceEngine {
public:
    InferenceEngine() {
        blockSize = 1024; // default block size

        // Set device
        if (torch::cuda::is_available()) {
            device = torch::Device(torch::kCUDA);
            logger->writeToLog("Using CUDA");
        } else {
            logger->writeToLog("Using CPU");
        }

#ifdef LOAD_FROM_PATH
        // Check if file exists
        std::ifstream f(model_path.c_str());
        if (!f.good()) {
            logger->writeToLog("Model file not found at: " + juce::String(model_path));
            throw std::runtime_error("Model file not found.");
        } else {
            f.close();
            logger->writeToLog("Model file found at: " + juce::String(model_path));
        }

        // Load the model checkpoint
        logger->writeToLog("Loading model from: " + juce::String(model_path));
        try {
            // For TorchScript models
            model = torch::jit::load(model_path);
        } catch (const c10::Error& e) {
            logger->writeToLog("Error loading model as TorchScript.");
            logger->writeToLog("Note: The .pth file needs to be converted to TorchScript format.");
            logger->writeToLog("Use torch.jit.script() or torch.jit.trace() in Python.");
            std::cerr << e.what() << std::endl;
            throw e;
        }
#else
        // Load model from binary data
        std::istringstream model_stream(std::string(reinterpret_cast<const char*>(BinaryData::PRISM_V2_traced_model_8bands_pth), static_cast<size_t>(BinaryData::PRISM_V2_traced_model_8bands_pthSize)));
        try {
            model = torch::jit::load(model_stream);
        } catch (const c10::Error& e) {
            logger->writeToLog("Error loading model from binary data as TorchScript.");
            logger->writeToLog("Note: The .pth file needs to be converted to TorchScript format.");
            logger->writeToLog("Use torch.jit.script() or torch.jit.trace() in Python.");
            std::cerr << e.what() << std::endl;
            throw e;
        }
#endif

        try {
            model.to(device);
        } catch (const c10::Error& e) {
            logger->writeToLog("Error moving model to device.");
            std::cerr << e.what() << std::endl;
            throw e;
        }

        try {
            model.eval();
        } catch (const c10::Error& e) {
            logger->writeToLog("Error setting model to eval mode.");
            std::cerr << e.what() << std::endl;
            throw e;
        }
        
        // Optimize model for inference
        try {
            logger->writeToLog("Optimizing model for inference...");
            model = torch::jit::optimize_for_inference(model);
            logger->writeToLog("Model optimization complete");
        } catch (const c10::Error& e) {
            logger->writeToLog("Warning: Could not optimize model for inference.");
            logger->writeToLog("Continuing with non-optimized model.");
            std::cerr << e.what() << std::endl;
            // Don't throw - continue with non-optimized model
        }
        
        logger->writeToLog("Model loaded successfully");

        // Initialize state and conditioning
        this->state = torch::randn({1, 1, 1022}, device);
        this->conditioning = torch::randn({1, N_BANDS, 8}, device);
    }
    ~InferenceEngine() = default;

    void prepareToPlay(int samplesPerBlock) {
        blockSize = samplesPerBlock;

        // Pre-allocate all tensors that will be used in real-time processing
        input_audio = torch::zeros({1, blockSize, 1}, device);
        
        // Pre-allocate inputs as IValue vector with exact size
        inputs.clear();
        inputs.reserve(3);
        inputs.push_back(torch::jit::IValue(input_audio));
        inputs.push_back(torch::jit::IValue(conditioning));
        inputs.push_back(torch::jit::IValue(state));
    }

    void runInference(const juce::AudioBuffer<float>& inBuffer, juce::AudioBuffer<float>& outBuffer) {
        torch::NoGradGuard no_grad;

        // Copy input audio data to tensor (this is unavoidable)
        const auto* readPtr = inBuffer.getReadPointer(0);
        std::memcpy(input_audio.data_ptr<float>(), readPtr, blockSize * sizeof(float));
        
        // Update IValue references in-place (avoid vector reallocation)
        inputs[0] = torch::jit::IValue(input_audio);
        inputs[1] = torch::jit::IValue(conditioning);
        inputs[2] = torch::jit::IValue(state);

        // Run inference
        auto output = model.forward(inputs);
        
        // Unpack tuple efficiently
        auto output_tuple = output.toTuple();
        auto& output_tensor = output_tuple->elements()[0].toTensor();
        auto& new_state = output_tuple->elements()[1].toTensor();
        
        // Update state in-place if possible, otherwise assign
        if (state.sizes() == new_state.sizes() && state.is_contiguous() && new_state.is_contiguous()) {
            state.copy_(new_state);  // In-place copy
        } else {
            state = new_state;  // Fallback to assignment
        }
        
        // Copy output to audio buffer
        auto* writePtr = outBuffer.getWritePointer(0);
        std::memcpy(writePtr, output_tensor.data_ptr<float>(), blockSize * sizeof(float));
    }

    void setConditioning(const std::array<std::array<float, LATENT_SIZE>, NUM_BANDS>& latents) {
        // Copy latents data to conditioning tensor
        float* condDataPtr = conditioning.data_ptr<float>();
        for (size_t i = 0; i < N_BANDS; ++i) {
            for (size_t j = 0; j < LATENT_SIZE; ++j) {
                condDataPtr[i * LATENT_SIZE + j] = latents[i][j];
            }
        }
    }

private:
    torch::jit::script::Module model;
    std::vector<torch::jit::IValue> inputs;
    torch::Tensor input_audio;
    torch::Tensor conditioning;
    torch::Tensor state;

    int blockSize = 1024;
    const int N_BANDS = 8;
    const std::string model_path = "C:/Users/cimil/Develop/paper-ideas/Ardan-JAES-25/MultibandDistortion/Source/Data/PRISM_traced_model_8bands.pth";
    
    torch::Device device{torch::kCPU};
    juce::Logger* logger = juce::Logger::getCurrentLogger();
};