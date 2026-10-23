#include "bambu_print.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace esphome::bambu_print
{

    using tessera::Font;
    namespace ui = tessera::ui;

    class DashboardTile : public tessera::Tile
    {
    public:
        explicit DashboardTile(BambuPrint *plugin) : plugin_(plugin) {}

        void create(const tessera::TileContext &context) override
        {
            width_ = context.width;
            height_ = context.height;

            const int margin = ui::px(8);
            const int arc_y = ui::px(16);
            const int arc_size = std::max(1, std::min(width_ - margin * 2, height_ * 42 / 100));
            arc_x_ = (width_ - arc_size) / 2;
            arc_y_ = arc_y;
            arc_size_ = arc_size;

            arc_ = lv_arc_create(context.parent);
            lv_obj_set_size(arc_, arc_size_, arc_size_);
            lv_obj_set_pos(arc_, arc_x_, arc_y_);
            lv_arc_set_rotation(arc_, 135);
            lv_arc_set_bg_angles(arc_, 0, 270);
            lv_arc_set_range(arc_, 0, 100);
            lv_arc_set_value(arc_, 0);
            lv_obj_remove_style(arc_, nullptr, LV_PART_KNOB);
            lv_obj_set_style_arc_width(arc_, ui::px(12), LV_PART_MAIN);
            lv_obj_set_style_arc_width(arc_, ui::px(12), LV_PART_INDICATOR);
            lv_obj_set_style_arc_rounded(arc_, true, LV_PART_MAIN);
            lv_obj_set_style_arc_rounded(arc_, true, LV_PART_INDICATOR);

            remaining_ = ui::label(context.parent, Font::BODY, theme::MUTED);
            lv_obj_set_width(remaining_, arc_size_);
            lv_obj_set_style_text_align(remaining_, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_pos(remaining_, arc_x_, arc_y_ + ui::px(8));

            stage_block_ = ui::block(context.parent, theme::CARD);
            lv_obj_set_width(stage_block_, arc_size_ * 62 / 100);
            lv_obj_set_height(stage_block_, ui::line_height(Font::BODY_LARGE) + ui::px(10));
            lv_obj_set_pos(stage_block_, arc_x_ + (arc_size_ - arc_size_ * 62 / 100) / 2,
                           arc_y_ + arc_size_ * 44 / 100);

            stage_ = ui::label(context.parent, Font::BODY_LARGE, theme::ON_ACCENT);
            lv_obj_set_width(stage_, arc_size_ * 58 / 100);
            lv_obj_set_style_text_align(stage_, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_pos(stage_, arc_x_ + (arc_size_ - arc_size_ * 58 / 100) / 2,
                           arc_y_ + arc_size_ * 44 / 100 + ui::px(5));

            progress_text_ = ui::label(context.parent, Font::DIGITS, theme::INK);
            lv_obj_set_width(progress_text_, arc_size_);
            lv_obj_set_style_text_align(progress_text_, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_pos(progress_text_, arc_x_, arc_y_ + arc_size_ - ui::line_height(Font::DIGITS) - ui::px(5));

            const int gap = ui::px(6);
            const int action_height = std::max(ui::mm(8), ui::line_height(Font::BODY_LARGE) + ui::px(12));
            action_y_ = height_ - action_height - margin;
            const int metrics_y = arc_y_ + arc_size_ + gap;
            const int metrics_height = std::max(1, action_y_ - metrics_y - gap);
            const int metrics_width = width_ - margin * 2;
            const int left_width = (metrics_width - gap) * 58 / 100;
            const int right_width = metrics_width - gap - left_width;
            const int left_x = margin;
            const int right_x = left_x + left_width + gap;

            temperatures_block_ = ui::block(context.parent, theme::CARD);
            lv_obj_set_size(temperatures_block_, left_width, metrics_height);
            lv_obj_set_pos(temperatures_block_, left_x, metrics_y);
            layers_block_ = ui::block(context.parent, theme::CARD);
            lv_obj_set_size(layers_block_, right_width, metrics_height);
            lv_obj_set_pos(layers_block_, right_x, metrics_y);

            nozzle_label_ = ui::label(context.parent, Font::BODY, theme::MUTED);
            bed_label_ = ui::label(context.parent, Font::BODY, theme::MUTED);
            nozzle_value_ = ui::label(context.parent, Font::VALUE, theme::INK);
            bed_value_ = ui::label(context.parent, Font::VALUE, theme::INK);
            layers_label_ = ui::label(context.parent, Font::BODY, theme::MUTED);
            layers_value_ = ui::label(context.parent, Font::VALUE, theme::INK);

            for (lv_obj_t *label : {nozzle_label_, bed_label_, nozzle_value_, bed_value_, layers_label_, layers_value_})
                lv_obj_set_width(label, left_width - ui::px(8));
            lv_obj_set_width(layers_label_, right_width - ui::px(8));
            lv_obj_set_width(layers_value_, right_width - ui::px(8));

            const int inset = ui::px(5);
            const int half_height = metrics_height / 2;
            lv_obj_set_pos(nozzle_label_, left_x + inset, metrics_y + inset);
            lv_obj_set_pos(nozzle_value_, left_x + inset, metrics_y + inset + ui::line_height(Font::BODY));
            lv_obj_set_pos(bed_label_, left_x + inset, metrics_y + half_height);
            lv_obj_set_pos(bed_value_, left_x + inset, metrics_y + half_height + ui::line_height(Font::BODY));
            lv_obj_set_pos(layers_label_, right_x + inset, metrics_y + inset);
            lv_obj_set_pos(layers_value_, right_x + inset, metrics_y + inset + ui::line_height(Font::BODY));

            action_width_ = (width_ - margin * 2 - gap * 3) / 4;
            const char *keys[] = {"pause", "resume", "stop", "light"};
            for (int index = 0; index < 4; ++index)
            {
                action_blocks_[index] = ui::block(context.parent, theme::CARD);
                lv_obj_set_size(action_blocks_[index], action_width_, action_height);
                lv_obj_set_pos(action_blocks_[index], margin + index * (action_width_ + gap), action_y_);
                action_labels_[index] = ui::label(context.parent, Font::BODY, theme::INK);
                lv_obj_set_width(action_labels_[index], action_width_);
                lv_obj_set_style_text_align(action_labels_[index], LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_set_pos(action_labels_[index], margin + index * (action_width_ + gap),
                               action_y_ + (action_height - ui::line_height(Font::BODY)) / 2);
                ui::set_text(action_labels_[index], plugin_->text(keys[index]));
            }

            ui::set_text(nozzle_label_, plugin_->text("nozzle"));
            ui::set_text(bed_label_, plugin_->text("bed"));
            ui::set_text(layers_label_, plugin_->text("layers"));
            on_theme();
        }

        void on_tick(uint32_t epoch) override
        {
            char value[32];
            const float progress = read(plugin_->progress());
            const int percent = std::isfinite(progress) ? std::clamp(static_cast<int>(std::lround(progress)), 0, 100) : 0;
            lv_arc_set_value(arc_, percent);
            std::snprintf(value, sizeof(value), "%d%%", percent);
            ui::set_text(progress_text_, value);

            const float minutes_value = read(plugin_->remaining_time());
            if (std::isfinite(minutes_value))
            {
                const int minutes = std::max(0, static_cast<int>(std::lround(minutes_value)));
                std::string time_text;
                if (minutes >= 60)
                {
                    time_text = tessera::fill(plugin_->text("remaining_hours"), "hours", std::to_string(minutes / 60));
                    time_text = tessera::fill(time_text, "minutes", std::to_string(minutes % 60));
                }
                else
                {
                    time_text = tessera::format(plugin_->text("remaining_minutes"), minutes);
                }
                ui::set_text(remaining_, time_text);
            }
            else
            {
                ui::set_text(remaining_, plugin_->text("no_value"));
            }

            const auto *stage = plugin_->stage();
            ui::set_text(stage_, stage != nullptr && stage->has_state() ? stage->state : plugin_->text("no_stage"));
            set_temperature(nozzle_value_, plugin_->nozzle_temperature());
            set_temperature(bed_value_, plugin_->bed_temperature());

            const float current_layer = read(plugin_->current_layer());
            const float total_layers = read(plugin_->total_layers());
            if (std::isfinite(current_layer) && std::isfinite(total_layers) && total_layers > 0)
            {
                std::snprintf(value, sizeof(value), "%.0f/%.0f", current_layer, total_layers);
            }
            else if (std::isfinite(current_layer))
            {
                std::snprintf(value, sizeof(value), "%.0f", current_layer);
            }
            else
            {
                std::snprintf(value, sizeof(value), "%s", plugin_->text("no_value").c_str());
            }
            ui::set_text(layers_value_, value);

            const auto *light = plugin_->light_state();
            const bool light_on = light != nullptr && light->has_state() && light->state == "on";
            ui::set_fill(action_blocks_[3], light_on ? tessera::Tone::ACCENT : tessera::Tone::NORMAL, theme::CARD);

            if (stop_pending_ && (!epoch || epoch > stop_pending_until_))
            {
                stop_pending_ = false;
                ui::set_text(action_labels_[2], plugin_->text("stop"));
            }
        }

        void on_theme() override
        {
            ui::set_color(remaining_, theme::MUTED);
            ui::set_color(stage_, theme::ON_ACCENT);
            ui::set_color(progress_text_, theme::INK);
            ui::set_color(nozzle_label_, theme::MUTED);
            ui::set_color(bed_label_, theme::MUTED);
            ui::set_color(nozzle_value_, theme::INK);
            ui::set_color(bed_value_, theme::INK);
            ui::set_color(layers_label_, theme::MUTED);
            ui::set_color(layers_value_, theme::INK);
            for (lv_obj_t *label : action_labels_)
                ui::set_color(label, theme::INK);
            ui::set_fill(stage_block_, tessera::Tone::ACCENT, theme::CARD);
            ui::set_fill(temperatures_block_, tessera::Tone::NORMAL, theme::CARD);
            ui::set_fill(layers_block_, tessera::Tone::NORMAL, theme::CARD);
            for (lv_obj_t *block : action_blocks_)
                ui::set_fill(block, tessera::Tone::NORMAL, theme::CARD);
            lv_obj_set_style_arc_color(arc_, ui::color(theme::TRACK), LV_PART_MAIN);
            lv_obj_set_style_arc_color(arc_, ui::color(theme::ACCENT), LV_PART_INDICATOR);
        }

        void on_tap() override
        {
            const auto point = tessera::tap_point();
            if (point.y < action_y_ || point.x < 0 || point.y >= height_)
                return;

            const int gap = ui::px(6);
            const int margin = ui::px(8);
            const int stride = action_width_ + gap;
            const int index = (point.x - margin) / stride;
            if (point.x < margin || index < 0 || index >= 4 || (point.x - margin) % stride >= action_width_)
                return;

            switch (index)
            {
            case 0:
                stop_pending_ = false;
                tessera::action("button.press", plugin_->pause_button());
                break;
            case 1:
                stop_pending_ = false;
                tessera::action("button.press", plugin_->resume_button());
                break;
            case 2:
                if (stop_pending_ && tessera::epoch() <= stop_pending_until_)
                {
                    tessera::action("button.press", plugin_->stop_button());
                    stop_pending_ = false;
                    ui::set_text(action_labels_[2], plugin_->text("stop"));
                }
                else
                {
                    stop_pending_ = true;
                    stop_pending_until_ = tessera::epoch() + 5;
                    ui::set_text(action_labels_[2], plugin_->text("confirm_stop"));
                }
                break;
            case 3:
                stop_pending_ = false;
                tessera::action("light.toggle", plugin_->chamber_light());
                break;
            }
        }

    private:
        static float read(const sensor::Sensor *sensor)
        {
            return sensor != nullptr && sensor->has_state() ? sensor->state : NAN;
        }

        void set_temperature(lv_obj_t *label, const sensor::Sensor *sensor)
        {
            const float temperature = read(sensor);
            char value[24];
            if (std::isfinite(temperature))
            {
                std::snprintf(value, sizeof(value), "%.0f C", temperature);
            }
            else
            {
                std::snprintf(value, sizeof(value), "%s", plugin_->text("no_value").c_str());
            }
            ui::set_text(label, value);
        }

        BambuPrint *plugin_;
        int width_ = 0;
        int height_ = 0;
        int arc_x_ = 0;
        int arc_y_ = 0;
        int arc_size_ = 0;
        int action_y_ = 0;
        int action_width_ = 0;
        bool stop_pending_ = false;
        uint32_t stop_pending_until_ = 0;
        lv_obj_t *arc_{};
        lv_obj_t *remaining_{};
        lv_obj_t *stage_block_{};
        lv_obj_t *stage_{};
        lv_obj_t *progress_text_{};
        lv_obj_t *temperatures_block_{};
        lv_obj_t *layers_block_{};
        lv_obj_t *nozzle_label_{};
        lv_obj_t *nozzle_value_{};
        lv_obj_t *bed_label_{};
        lv_obj_t *bed_value_{};
        lv_obj_t *layers_label_{};
        lv_obj_t *layers_value_{};
        lv_obj_t *action_blocks_[4]{};
        lv_obj_t *action_labels_[4]{};
    };

    void BambuPrint::setup()
    {
        add_tile("dashboard", [this]()
                 { return new DashboardTile(this); });
    }

} // namespace esphome::bambu_print