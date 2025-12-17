from glob import glob
import os

wavs = glob("example*/*.mp3", recursive=True)
# folders = set()
# for wav in wavs:
#     folders.add(os.path.dirname(wav))
# folders = sorted(list(folders))

# print(folders)

get_info_file = lambda path: os.path.join(os.path.dirname(path), "band_settings_8bands.txt")
for wav in wavs:
    info_file = get_info_file(wav)
    assert os.path.exists(info_file), f"Info file not found for {wav}"
    bandstrs = ['']*8
    with open(info_file, "r") as f:
        lines = f.readlines()
        for line in lines:
            if "Band " in line and " pedal=" in line:
                band = int(line.split("Band ")[1].split(":")[0])
                pedal = line.split(" pedal=")[1].strip()
                gain = line.split(" gain=")[1].split(" ")[0].split(",")[0].split("\n")[0]
                tone = line.split(" tone=")[1].split(" ")[0].split(",")[0].split("\n")[0]
                bandstrs[band] = f"B{band}P{pedal[0]}G{gain}T{tone}"
    bandinfo = "_".join(bandstrs)
    newname = f"{os.path.splitext(os.path.basename(wav))[0]}_{bandinfo}{os.path.splitext(wav)[1]}"
    newname = newname.replace("_prism8", "")

    print(f"Renaming {wav} to {newname}")
    os.rename(wav, os.path.join(os.path.dirname(wav), newname))
