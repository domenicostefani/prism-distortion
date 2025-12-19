import os
import re
from glob import glob

def parse_band_settings(filename):
    """
    Parse band settings from filename.
    Example: bass_B0PfG5T5_B1PfG4T4_B2PfG4T4_B3PfG3T3_B4PfG3T3_B5PfG2T2_B6PfG2T2_B7PfG1T1.mp3
    Returns: [('f', 5, 5), ('f', 4, 4), ...]
    """
    # Extract all band configurations
    band_pattern = r'B(\d)P([kfr])G(\d)T(\d)'
    matches = re.findall(band_pattern, filename)
    
    if len(matches) != 8:
        raise ValueError(f"Expected 8 bands in filename, found {len(matches)}: {filename}")
    
    band_settings = []
    for band_idx, pedal_type, gain, tone in matches:
        band_settings.append((pedal_type, int(gain), int(tone)))
    
    return band_settings


def get_source_name(wet_filename):
    """
    Extract the source audio name from wet filename.
    Example: bass_B0PfG5T5_B1PfG4T4_...mp3 -> bass
    """
    # Remove extension and extract everything before the first _B
    name = os.path.basename(wet_filename)
    name = name.replace('.mp3', '')
    source_match = re.match(r'^(.+?)_B\d', name)
    if source_match:
        return source_match.group(1)
    return name


def get_table_row(idx, dry_path, dry_amp_path, wet_path, wet_amp_path, band_settings):
    """
    Generate a table row for multiband effect demo.
    
    Args:
        idx: Row number
        dry_path: Path to dry DI signal
        dry_amp_path: Path to dry signal with amplifier
        wet_path: Path to wet DI signal
        wet_amp_path: Path to wet signal with amplifier
        band_settings: List of 8 tuples, each containing (type, gain, tone)
                      where type is 'k' (Overdrive), 'f' (Fuzz), or 'r' (Distortion)
    """
    assert len(band_settings) == 8, "Must have exactly 8 bands"
    
    # Type mapping and colors
    type_map = {'k': 'OD', 'f': 'Fuzz', 'r': 'Dist'}
    type_colors = {'k': '#569bfd', 'f': '#ec2215', 'r': '#faeb32'}
    
    # Build band settings HTML - 8 bands as columns
    bands_html = '<table class="progtab text-center table table-borderless align-middle" style="width: 100%; table-layout: fixed;">'
        
    band_labels = [
        "<b>B1<br>(~40-500Hz)</b>",
        "<b>B2<br>(~0.5 - 1kHz)</b>",
        "<b>B3<br>(~1 - 1.6kHz)</b>",
        "<b>B4<br>(~1.6 - 2.7kHz)</b>",
        "<b>B5<br>(~2.7 - 4.5kHz)</b>",
        "<b>B6<br>(~4.5 - 7.4kHz)</b>",
        "<b>B7<br>(~7.4 - 12kHz)</b>",
        "<b>B8<br>(~12 - 20kHz)</b>",
    ]

    # Band headers row
    bands_html += '<tr>'
    bands_html += f'<td class="align-middle left" style="width: auto;"><b>Frequency Bands:</b></td>'
    for band_idx in range(1, 9):
        bands_html += f'<td class="align-middle" style="font-size: 0.85em;">{band_labels[band_idx-1]}</td>'
    bands_html += '</tr>'

    # Type selector row
    bands_html += '<tr>'
    bands_html += '<td class="align-middle left" style="font-size: 0.85em; font-weight: bold; white-space: nowrap;">Effect</td>'
    for band_idx, (band_type, gain, tone) in enumerate(band_settings, 1):
        assert band_type in ['k', 'f', 'r'], f"Band type must be 'k', 'f', or 'r', got {band_type}"
        assert 0 <= gain <= 5, f"Gain must be between 0 and 5, got {gain}"
        assert 0 <= tone <= 5, f"Tone must be between 0 and 5, got {tone}"
        
        type_name = type_map[band_type]
        type_color = type_colors[band_type]
        
        # text_color white if darkish background, black otherwise
        hsl = tuple(int(type_color.lstrip('#')[i:i+2], 16) for i in (0, 2, 4))
        brightness = (hsl[0] * 299 + hsl[1] * 587 + hsl[2] * 114) / 1000
        text_color = '#FFFFFF' if brightness < 150 else '#000000'   
        print("brightness:", int(brightness), "text_color:", text_color)

        bands_html += f'''
        <td class='align-middle'>
            <div class="type-selector">
                <div class="type-bar" style="background-color:{type_color}; color:{text_color}; width:100%" title="{type_name}">{type_name}</div>
            </div>
        </td>'''
    bands_html += '</tr>'

    # Gain row
    bands_html += '<tr>'
    bands_html += '<td class="align-middle left" style="font-size: 0.85em; font-weight: bold; white-space: nowrap;">Gain</td>'
    for band_idx, (band_type, gain, tone) in enumerate(band_settings, 1):
        bands_html += f'''
        <td class='align-middle' style=" margin-bottom: 1rem;">
            <div class="progress" style="height: 1rem;">
                <div class="progress-bar bg-info" style="width: {gain/5*100}%; color:#000;">G{gain}/5</div>
            </div>
        </td>'''
    bands_html += '</tr>'

    # Tone row
    bands_html += '<tr>'
    bands_html += '<td class="align-middle left" style="font-size: 0.85em; font-weight: bold; white-space: nowrap;">Tone</td>'
    for band_idx, (band_type, gain, tone) in enumerate(band_settings, 1):
        bands_html += f'''
        <td class='align-middle'>
            <div class="progress" style="height: 15px;">
                <div class="progress-bar bg-warning" style="width: {tone/5*100}%; color:#000;">T{tone}/5</div>
            </div>
        </td>'''
    bands_html += '</tr>'

    bands_html += '</table>'

    res = f'''
    <tr>
        <td class="align-middle">{idx}</td>
        <td class="align-middle" colspan="2">
            {bands_html}
        </td>
    </tr>
    <tr>
        <td></td>
        <td class="align-middle">
            <b>Dry+Amp:</b> <a href="{dry_amp_path}">{os.path.basename(dry_amp_path)}</a><br>
            <audio controls>
                <source src="{dry_amp_path}" type="audio/mp3">
            </audio><br>
            <b>Dry DI:</b> <a href="{dry_path}">{os.path.basename(dry_path)}</a><br>
            <audio controls>
                <source src="{dry_path}" type="audio/mp3">
            </audio>
        </td>
        <td class="align-middle">
            <b>Wet+Amp:</b> <a href="{wet_amp_path}">{"_".join(os.path.basename(wet_amp_path).split('_')[:2])}_wet</a><br>
            <audio controls>
                <source src="{wet_amp_path}" type="audio/mp3">
            </audio><br>
            <b>Wet DI:</b> <a href="{wet_path}">{"_".join(os.path.basename(wet_path).split('_')[:1])}</a><br>
            <audio controls>
                <source src="{wet_path}" type="audio/mp3">
            </audio>
        </td>
    </tr>
    '''
    
    return res

HEADER = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="Multiband neural audio effect demos">
    <title>Prism Demos</title>
    
    <link rel="stylesheet"
      href="https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css"
      integrity="sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm"
      crossorigin="anonymous">

    <style>
    
        .hero-section {
        background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        color: white;
        padding: 2rem 0;
        }

        .table-borderless td, .table-borderless th {
            border: none;
        }
        .progtab {
            margin: 0 auto;
            padding: 0;
            width: 100%;
            min-width: 800px; /* Force minimum width to ensure scrolling */
        }
        .progtab tr {
            margin: 0;
            padding: 2px 0;
        }
        .progtab td {
            margin: 0;
            padding: 2px 5px;
        }
        .left {
            width: 30%;
            text-align: left;
        }
        .type-selector {
            position: relative;
            height: 20px;
            border-radius: 4px;
            border: 1px solid #ddd;
        }
        .type-bar {
            position: absolute;
            top: -3px;
            width: 33.33%;
            height: 26px;
            border: 2px solid #000;
            border-radius: 4px;
            transition: left 0.3s;
        }
        .progress {
            background-color: #e9ecef;
        }
        audio {
            width: 100%;
            max-width: 300px;
        }
        
        /* Critical fix for horizontal scrolling */
        .table-scroll-wrapper {
            overflow-x: auto;
            -webkit-overflow-scrolling: touch; /* Smooth scrolling on iOS */
        }
        
        /* Smartphones */
        @media (max-width: 600px) {
            .hero-section h1 {
                font-size: 2rem;
            }
        }
    </style>
</head>

<body>
    <div class="hero-section text-center">
      <div class="container">
        <div class="mb-1"
          style="display: flex; align-items: center; justify-content: center; gap: 1rem;">
          <img src="../logo.svg" alt="Prism Logo" class="logo"
            style="width: 4rem;">
          <h1 class="display-3 font-weight-bold mb-3">All Prism demos</h1>
        </div>
        <div class="mt-4">
          <a class="btn btn-outline-light btn-lg mx-2 mb-2"
            href="../index.html">Back to Homepage</a>
        </div>
      </div>
    </div>

    <div class="container mt-4" style="max-width: 800px; margin-left: auto; margin-right: auto;">
        <p>This page contains audio demos fror the Prism multiband effect</p>
        <p>All the rows first show the settings for the 8 frequency bands, which comprise per-band effect type (Fuzz, Overdrive, Distortion), Gain (0-5) and Tone (0-5)</p>
        
        <p>
            Then each example contains 4 audio files: the top row shows the <b>Dry (no Prism)</b> and <b>Wet (with Prism)</b> signals, where an amplifier with cabinet simulation was used.
            The bottom row contains the DI input and straight pedal output (<b>Dry DI</b>,<b>Wet DI</b>).
        </p>
    </div>

    <div class="container mt-4" style="max-width: 1400px; margin-left: auto; margin-right: auto;">
        <div id="examples" class="section mt-4">
            <div class="table-scroll-wrapper text-center">
                <table class="text-center table">
                    <tbody>
"""

FOOTER = """
                    </tbody>
                </table>
            </div>
        </div>
    </div>

    
    <!-- Footer -->
    <hr>
    <footer class="text-center py-4">
        <p>
        © 2025 <a href="https://github.com/return-nihil">Ardan Dal Rì</a>
        (return_nihil) &
        <a href="http://domenicostefani.com">Domenico Stefani</a> (OnyxDSP)
        </p>
    </footer>

    <script src="https://code.jquery.com/jquery-3.5.1.slim.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/popper.js@1.16.0/dist/umd/popper.min.js"></script>
    <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/js/bootstrap.min.js"></script>
</body>
</html>
"""


# Main script
if __name__ == "__main__":
    # Set paths
    dry_folder = 'dry'
    wet_folder = 'wet'  # This is your wet folder based on the document
    
    # Get all wet files
    wet_files = sorted(glob(os.path.join(wet_folder, '*.mp3')))
    print(f"Found {len(wet_files)} wet files.")
    
    # Filter out the amp_project.rpp if it got picked up
    wet_files = [f for f in wet_files if '_B0P' in f]
    
    file_lines = []
    file_lines.append(HEADER)

    prev_filename = ""
    indexlinkslines = []
    
    
    output_lines = []
    for idx, wet_path in enumerate(wet_files, 1):
        try:
            # Parse band settings from filename
            band_settings = parse_band_settings(os.path.basename(wet_path))
            
            # Get source audio name
            source_name = get_source_name(wet_path)
            
            # Build dry paths
            dry_path = os.path.join(dry_folder, f'{source_name}.mp3')
            dry_amp_path = os.path.join(dry_folder, 'amp', f'amp_{source_name}.mp3')
            
            # Build wet amp path (wet files are already in wet folder, amp version in wet/amp subfolder)
            wet_amp_path = os.path.join(wet_folder, 'amp', f'amp_{os.path.basename(wet_path)}')
            
            # Verify all files exist
            assert os.path.exists(dry_path), f"Dry file not found: {dry_path}"
            assert os.path.exists(dry_amp_path), f"Dry amp file not found: {dry_amp_path}"
            assert os.path.exists(wet_path), f"Wet file not found: {wet_path}"
            assert os.path.exists(wet_amp_path), f"Wet amp file not found: {wet_amp_path}"
            
            if dry_path != prev_filename:
                # print(f"Processing new source: {source_name}")
                row = f'''
    <tr><td colspan="3"><hr></td></tr>
    <tr>
        <td colspan="3" class="text-center" id="{source_name}"><h3>{source_name.capitalize()}</h3><a style="size: 0.5rem;" href="#index">Back to top</a></td>
    </tr>
                '''
                output_lines.append(row)

                # indexlinkslines
                indexlinkslines.append(f'<a href="#{source_name}">{source_name.capitalize()}</a>')

                prev_filename = dry_path
            # Generate table row
            row = get_table_row(
                idx,
                dry_path,
                dry_amp_path,
                wet_path,
                wet_amp_path,
                band_settings
            )
            output_lines.append(row)
            
            print(f"✓ Processed: {os.path.basename(wet_path)}")
            
        except Exception as e:
            print(f"✗ Error processing {wet_path}: {e}")
            continue
    
    file_lines.append('''
<h2 id="index">Index</h2>
''')
    file_lines.append('<br>\n'.join(indexlinkslines))
    file_lines.append('<br><br>')
    file_lines.extend(output_lines)
    file_lines.append(FOOTER)
    
    # Write to file
    with open('demos-multiband.html', 'w', encoding='utf-8') as f:
        f.writelines(file_lines)
    
    # print(f"\n✓ Generated demos-multiband.html successfully with {idx} demos!")