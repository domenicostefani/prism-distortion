import csv
import ast

# Map label to effect index
effect_map = {
    "riot": 0,
    "fuzz": 1,
    "kingoftone": 2
}
reverse_map = {v: k for k, v in effect_map.items()}

better_name_map = {
    "riot": "Distortion (Suhr Riot)",
    "fuzz": "Fuzz (JAM Red Muck)",
    "kingoftone": "Overdrive (King of Tone)"
}

# Prepare 3×6×6×8 array, filled with zeros
data = [[[[0.0 for _ in range(8)] for _ in range(6)] for _ in range(6)] for _ in range(3)]

# Load CSV
with open("PRISM_vae_latents.csv", newline="") as f:
    reader = csv.DictReader(f)
    for row in reader:
        effect = effect_map[row["label"]]
        g = int(row["g"])
        t = int(row["t"])
        latents = ast.literal_eval(row["latents"])  # safely parse the string list

        # store
        data[effect][g][t] = latents

# ----------- BUILD C++ OUTPUT -----------
def fmt_vec(v):
    return "{" + ",".join(f"{x}f" for x in v) + "}"


prefix = r"""
#pragma once

#include <JuceHeader.h>

class VAELatentDataFrame
{
public:
    void getLatent(int effectType, int gain, int tone, std::array<float,8>& latentOut)
    {
        // Effect type: 0,1,2 (Distortion, Fuzz, Overdrive)
        // Gain: 0,2,4,6,8,10
        // Tone: 0,2,4,6,8,10

        int gainIndex = gain / 2;
        int toneIndex = tone / 2;

        if (effectType < 0 || effectType > 2 ||
            gainIndex < 0 || gainIndex > 5 ||
            toneIndex < 0 || toneIndex > 5)
        {
            // Invalid parameters, return zero latent
            latentOut.fill(0.0f);
            return;
        }
        latentOut = data[effectType][gainIndex][toneIndex];
    }

    VAELatentDataFrame() :
"""

tab = "            "
cpp = prefix+"\n"+tab+"data{{\n"

for eff in range(3):
    cpp += tab+f"    // Effect {eff}:{better_name_map[reverse_map[eff]]}\n"+tab+"    {{\n"
    for g in range(6):
        cpp += tab+f"        // Gain {g*2} - {better_name_map[reverse_map[eff]]}\n"+tab+"        {{\n"
        for t in range(6):
            cpp += tab+f"            {fmt_vec(data[eff][g][t])}, // Tone {t*2}\n"
            assert len(data[eff][g][t]) == 8, "Latent vector must have 8 elements"
        cpp += tab+"        }},\n"
    cpp += tab+"    }},\n"

cpp += tab+"}}"

suffix = r"""  
    {}
    ~VAELatentDataFrame() = default;

private:
    std::array<std::array<std::array<std::array<float, 8>, 6>, 6>, 3> data;
    // Data initialization can be done here or loaded from an external source
};
"""
cpp += suffix

# print(cpp)

with open("VAEdataframe.h", "w") as f:
    f.write(cpp)
    print("Wrote ./VAEdataframe.h\nPlease copy it over to Source/VAEDateframe.h")
