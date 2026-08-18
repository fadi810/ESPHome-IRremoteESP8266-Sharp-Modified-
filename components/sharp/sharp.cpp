#include "sharp.h"

namespace esphome
{
    namespace sharp
    {

        static const char *const TAG = "sharp.climate";

        void SharpClimate::set_model(const Model model)
        {
            this->ac_.setModel((sharp_ac_remote_model_t)model);
        }

        void SharpClimate::setup()
        {
            climate_ir::ClimateIR::setup();
            this->apply_state();
        }

        void SharpClimate::control(const climate::ClimateCall &call)
        {
            if (call.has_custom_preset())
            {
                auto preset = call.get_custom_preset();

                bool changed = !this->has_custom_preset() ||
                               preset != this->get_custom_preset();

                if (changed)
                {
                    this->timer_changed_ = true;
                    this->set_custom_preset_(preset);
                    ESP_LOGI(TAG, "Timer preset changed to: %s", preset.c_str());
                }
            }

            climate_ir::ClimateIR::control(call);
        }

        void SharpClimate::transmit_state()
        {
            this->cancel_timeout("timer");

            if (this->mode == climate::CLIMATE_MODE_OFF)
            {
                this->timer_changed_ = false;
                this->apply_state();
                this->send();
                return;
            }

            this->apply_state();
            this->send_debounced();

            if (!this->timer_changed_ || !this->has_custom_preset())
            {
                return;
            }

            auto preset = this->get_custom_preset();

            if (preset == "Timer Off")
            {
                this->timer_changed_ = false;
                return;
            }

            this->set_timeout("timer", 1000, [this]()
                              {
        this->send_timer();
        this->timer_changed_ = false; });
        }

        void SharpClimate::send()
        {
            uint8_t *message = this->ac_.getRaw();

            sendGeneric(
                kSharpAcHdrMark, kSharpAcHdrSpace,
                kSharpAcBitMark, kSharpAcOneSpace,
                kSharpAcBitMark, kSharpAcZeroSpace,
                kSharpAcBitMark, kSharpAcGap,
                message, kSharpAcStateLength,
                38000);
        }

        void SharpClimate::apply_state()
        {
            this->ac_.setTimer(false, false, 0);

            if (this->mode == climate::CLIMATE_MODE_OFF)
            {
                this->ac_.off();
                ESP_LOGI(TAG, "State: OFF");
                return;
            }

            this->ac_.setTemp(this->target_temperature);

            switch (this->mode)
            {
            case climate::CLIMATE_MODE_AUTO:
                this->ac_.setMode(kSharpAcAuto);
                break;

            case climate::CLIMATE_MODE_HEAT:
                this->ac_.setMode(kSharpAcHeat);
                break;

            case climate::CLIMATE_MODE_COOL:
                this->ac_.setMode(kSharpAcCool);
                break;

            case climate::CLIMATE_MODE_DRY:
                this->ac_.setMode(kSharpAcDry);
                break;

            case climate::CLIMATE_MODE_FAN_ONLY:
                this->ac_.setMode(kSharpAcFan);
                break;

            default:
                break;
            }

            if (this->fan_mode.has_value())
            {
                switch (this->fan_mode.value())
                {
                case climate::CLIMATE_FAN_AUTO:
                    this->ac_.setFan(kSharpAcFanAuto);
                    break;

                case climate::CLIMATE_FAN_LOW:
                    this->ac_.setFan(kSharpAcFanMin);
                    break;

                case climate::CLIMATE_FAN_MEDIUM:
                    this->ac_.setFan(kSharpAcFanMed);
                    break;

                case climate::CLIMATE_FAN_HIGH:
                    this->ac_.setFan(kSharpAcFanMax);
                    break;

                default:
                    break;
                }
            }

            switch (this->swing_mode)
            {
            case climate::CLIMATE_SWING_OFF:
                this->ac_.setSwingV(kSharpAcSwingVOff);
                break;

            case climate::CLIMATE_SWING_VERTICAL:
                this->ac_.setSwingV(0b111);
                break;

            default:
                break;
            }

            this->ac_.on();

            ESP_LOGI(TAG, "State: %s", this->ac_.toString().c_str());
        }

        void SharpClimate::apply_timer()
        {
            if (!this->has_custom_preset())
            {
                return;
            }

            auto preset = this->get_custom_preset();

            if (preset == "Timer 30m")
            {
                this->ac_.setTimer(true, false, 30);
                ESP_LOGI(TAG, "Timer: 30m");
            }
            else if (preset == "Timer 1h")
            {
                this->ac_.setTimer(true, false, 60);
                ESP_LOGI(TAG, "Timer: 1h");
            }
            else if (preset == "Timer 1.5h")
            {
                this->ac_.setTimer(true, false, 90);
                ESP_LOGI(TAG, "Timer: 1.5h");
            }
            else if (preset == "Timer 2h")
            {
                this->ac_.setTimer(true, false, 120);
                ESP_LOGI(TAG, "Timer: 2h");
            }
        }

        void SharpClimate::send_timer()
        {
            this->apply_timer();

            uint8_t *message = this->ac_.getRaw();

            sendGeneric(
                kSharpAcHdrMark, kSharpAcHdrSpace,
                kSharpAcBitMark, kSharpAcOneSpace,
                kSharpAcBitMark, kSharpAcZeroSpace,
                kSharpAcBitMark, kSharpAcGap,
                message, kSharpAcStateLength,
                38000);
        }

    } // namespace sharp
} // namespace esphome
