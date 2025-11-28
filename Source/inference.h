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
            // std::cerr << "Model file not found at: " << model_path << std::endl;
            logger->writeToLog("Model file not found at: " + juce::String(model_path));
            throw std::runtime_error("Model file not found.");
        } else {
            f.close();
            // std::cout << "Model file found at: " << model_path << std::endl;
            logger->writeToLog("Model file found at: " + juce::String(model_path));
        }

        // Load the model checkpoint
        // std::cout << "Loading model from: " << model_path << std::endl;
        logger->writeToLog("Loading model from: " + juce::String(model_path));
        try {
            // For TorchScript models
            model = torch::jit::load(model_path);
        } catch (const c10::Error& e) {
            // std::cerr << "Error loading model as TorchScript." << std::endl;
            // std::cerr << "Note: The .pth file needs to be converted to TorchScript format." << std::endl;
            // std::cerr << "Use torch.jit.script() or torch.jit.trace() in Python." << std::endl;
            logger->writeToLog("Error loading model as TorchScript.");
            logger->writeToLog("Note: The .pth file needs to be converted to TorchScript format.");
            logger->writeToLog("Use torch.jit.script() or torch.jit.trace() in Python.");
            std::cerr << e.what() << std::endl;
            throw e;
        }
#else
        // Load model from binary data (BinaryData::PRISM_traced_model_8bands_pth, BinaryData::PRISM_traced_model_8bands_pthSize)
        // Need std::istream from memory buffer
        // V1
        // std::istringstream model_stream(std::string(reinterpret_cast<const char*>(BinaryData::PRISM_traced_model_8bands_pth), BinaryData::PRISM_traced_model_8bands_pthSize));
        // V2
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
            // std::cerr << "Error moving model to device." << std::endl;
            logger->writeToLog("Error moving model to device.");
            std::cerr << e.what() << std::endl;
            throw e;
        }

        try {
            model.eval();
        } catch (const c10::Error& e) {
            // std::cerr << "Error setting model to eval mode." << std::endl;
            logger->writeToLog("Error setting model to eval mode.");
            std::cerr << e.what() << std::endl;
            throw e;
        }
        
        // std::cout << "Model loaded successfully" << std::endl;
        logger->writeToLog("Model loaded successfully");

        this->state = torch::randn({1, 1, 1022}, device);
        this->conditioning = torch::randn({1, N_BANDS, 8}, device);
    }
    ~InferenceEngine() = default;

    void prepareToPlay(int samplesPerBlock) {
        blockSize = samplesPerBlock;

        input_audio = torch::randn({1, blockSize, 1}, device);

        // std::cout << "Input audio shape: " << input_audio.sizes() << std::endl;
        // std::cout << "Conditioning shape: " << conditioning.sizes() << std::endl;
        // std::cout << "State shape: " << state.sizes() << std::endl;
        // logger->writeToLog("Input audio shape: " + juce::String(input_audio.sizes()));
        // logger->writeToLog("Conditioning shape: " + juce::String(conditioning.sizes()));
        // logger->writeToLog("State shape: " + juce::String(state.sizes()));

        inputs.resize(3);
    }

    void runInference(const juce::AudioBuffer<float>& inBuffer, juce::AudioBuffer<float>& outBuffer) {
        torch::NoGradGuard no_grad;

        // Copy audio inBuffer data to input_audio tensor
        auto* readPtr = inBuffer.getReadPointer(0);
        std::memcpy(input_audio.data_ptr(), readPtr, blockSize * sizeof(float));
        // Construct inputs vector
        inputs[0] = input_audio;
        inputs[1] = conditioning;
        inputs[2] = state;

        // Run the model
        output = model.forward(inputs);

        output_tuple = output.toTuple(); // TODO: understand type and move to members
        output_tensor = output_tuple->elements()[0].toTensor();
        // std::cout << "Output shape: " << output_tensor.sizes() << std::endl;
        // if (output_tuple->elements().size() > 1) {
        // std::cout << "Additional outputs (hidden states) also returned" << std::endl;
        // save to state variable
        returned_state = output_tuple->elements()[1].toTensor();
        // std::cout << "Returned state shape: " << returned_state.sizes() << std::endl;
        state = returned_state;
        // }

        // Copy output tensor data to outBuffer
        auto* writePtr = outBuffer.getWritePointer(0);
        std::memcpy(writePtr, output_tensor.data_ptr(), blockSize * sizeof(float));
    }

    void setConditioning(const std::array<std::array<float, LATENT_SIZE>, NUM_BANDS>& latents) {
        // Copy latents data to conditioning tensor
        float* condDataPtr = conditioning.data_ptr<float>();
        for (size_t i = 0; i < N_BANDS; ++i) {  // TODO: Ardan è giusto? Blocchi ti 8 latent float da 8
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
    

    torch::jit::IValue output; // output of model.forward(inputs);
    c10::intrusive_ptr<c10::ivalue::Tuple> output_tuple;
    torch::Tensor returned_state;      // first element of output_tuple
    torch::Tensor output_tensor;      // second element of output_tuple

    int blockSize = 1024;
    const int N_BANDS = 8;
    const std::string model_path = "C:/Users/cimil/Develop/paper-ideas/Ardan-JAES-25/MultibandDistortion/Source/Data/PRISM_traced_model_8bands.pth"; // TODO: Load from binary
    
    torch::Device device{torch::kCPU};
    // Juce Logger
    juce::Logger* logger = juce::Logger::getCurrentLogger();
};

