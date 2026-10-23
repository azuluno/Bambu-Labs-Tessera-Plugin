# Bambu Lab Tessera Plugin

A Tessera tile for a Bambu Lab printer. It shows print progress, stage, time remaining, nozzle and bed temperatures, and layer counts. Pause, resume, stop, and chamber-light controls call Home Assistant actions.

## Set up

### Requirements

- Tessera's plugin-capable app and firmware. The manifest targets plugin API `0.14`, currently available on Tessera's development branch; use a matching app/core before adding this plugin.
- The Bambu Lab Home Assistant integration, with the printer entities enabled.
- A Home Assistant template sensor that combines the printer readings as attributes. Tessera plugin inputs are limited, and one entity input lets the tile receive all readings together.
- Home Assistant must be able to control the printer. Bambu firmware/LAN mode can restrict write actions; sensor readings may still work when controls do not.

## Home Assistant data sensor

Add a template sensor to `configuration.yaml` (or your templates package). Replace entity IDs with the ones shown under **Developer Tools > States** for your printer. Names vary by printer and integration version.

```yaml
template:
  - sensor:
      - name: "R2 A1 Tessera Data"
        unique_id: r2_a1_tessera_data
        state: "{{ states('sensor.r2_a1_panda_print_status') }}"
        attributes:
          progress: "{{ states('sensor.r2_a1_panda_print_progress') | float(0) }}"
          current_stage: "{{ states('sensor.r2_a1_panda_current_stage') }}"
          remaining_time: "{{ states('sensor.r2_a1_panda_remaining_time') | float(0) }}"
          nozzle_temperature: "{{ states('sensor.r2_a1_panda_nozzle_temperature') | float(0) }}"
          bed_temperature: "{{ states('sensor.r2_a1_panda_bed_temperature') | float(0) }}"
          current_layer: "{{ states('sensor.r2_a1_panda_current_layer') | int(0) }}"
          total_layers: "{{ states('sensor.r2_a1_panda_total_layer_count') | int(0) }}"
```

Reload template entities, then confirm that the resulting sensor has all seven attributes. The current Bambu Lab integration commonly names the layer entities `current_layer` and `total_layers`; if your `sensor.r2_a1_layer_count` is the layer value you want, use it for `current_layer` and keep `total_layers` optional.

## Add the plugin

1. In Tessera, open **Plugins > Add with a link** and add this GitHub repository.
2. Add the plugin to the target screen. Select the aggregate data sensor, the pause/resume/stop buttons, and the chamber light when prompted, then build the screen.
3. In **Layout**, add the **Bambu Printer** plugin tile. Place it in a 2x3 or larger grid area.

The Bambu integration exposes pause, resume, and stop as `button` entities. The chamber light is a `light` entity; on some models, including the A1 Mini, it may be named as a camera light. Choose the actual entities from the Tessera input selectors rather than relying on the sample names.

Stop requires a second tap within five seconds. The manifest declares `button.press` and `light.toggle` as the only Home Assistant actions used by the plugin.

## Develop and test

The plugin files are at the repository root so Tessera can add the repository directly. Push changes to GitHub and use Tessera's **Build again** action after it sees the update. The Tessera plugin bench can also validate the manifest and render the tile across screen sizes; its preview setup runs under Linux or WSL 2 on Windows.
