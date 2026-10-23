#pragma once

#include <string>
#include <utility>

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/smart_display/plugin_api.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome::bambu_print
{

    class BambuPrint : public Component, public tessera::Plugin
    {
    public:
        void setup() override;
        float get_setup_priority() const override { return setup_priority::DATA; }

        void set_progress(sensor::Sensor *value) { progress_ = value; }
        void set_remaining_time(sensor::Sensor *value) { remaining_time_ = value; }
        void set_nozzle_temperature(sensor::Sensor *value) { nozzle_temperature_ = value; }
        void set_bed_temperature(sensor::Sensor *value) { bed_temperature_ = value; }
        void set_current_layer(sensor::Sensor *value) { current_layer_ = value; }
        void set_total_layers(sensor::Sensor *value) { total_layers_ = value; }
        void set_stage(text_sensor::TextSensor *value) { stage_ = value; }
        void set_light_state(text_sensor::TextSensor *value) { light_state_ = value; }
        void set_pause_button(std::string value) { pause_button_ = std::move(value); }
        void set_resume_button(std::string value) { resume_button_ = std::move(value); }
        void set_stop_button(std::string value) { stop_button_ = std::move(value); }
        void set_chamber_light(std::string value) { chamber_light_ = std::move(value); }

        sensor::Sensor *progress() const { return progress_; }
        sensor::Sensor *remaining_time() const { return remaining_time_; }
        sensor::Sensor *nozzle_temperature() const { return nozzle_temperature_; }
        sensor::Sensor *bed_temperature() const { return bed_temperature_; }
        sensor::Sensor *current_layer() const { return current_layer_; }
        sensor::Sensor *total_layers() const { return total_layers_; }
        text_sensor::TextSensor *stage() const { return stage_; }
        text_sensor::TextSensor *light_state() const { return light_state_; }
        const std::string &pause_button() const { return pause_button_; }
        const std::string &resume_button() const { return resume_button_; }
        const std::string &stop_button() const { return stop_button_; }
        const std::string &chamber_light() const { return chamber_light_; }

    private:
        sensor::Sensor *progress_{};
        sensor::Sensor *remaining_time_{};
        sensor::Sensor *nozzle_temperature_{};
        sensor::Sensor *bed_temperature_{};
        sensor::Sensor *current_layer_{};
        sensor::Sensor *total_layers_{};
        text_sensor::TextSensor *stage_{};
        text_sensor::TextSensor *light_state_{};
        std::string pause_button_;
        std::string resume_button_;
        std::string stop_button_;
        std::string chamber_light_;
    };

} // namespace esphome::bambu_print