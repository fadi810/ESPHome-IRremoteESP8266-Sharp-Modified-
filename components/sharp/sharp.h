#pragma once

#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/ir_remote_base/ir_remote_base.h"
#include "ir_Sharp.h"

namespace esphome
{
    namespace sharp
    {

        enum Model
        {
            A907 = sharp_ac_remote_model_t::A907,
            A705 = sharp_ac_remote_model_t::A705,
            A903 = sharp_ac_remote_model_t::A903,
        };

        class SharpClimate : public ir_remote_base::IrRemoteBase
        {
        public:
            SharpClimate()
                : IrRemoteBase(
                      20,
                      30,
                      1.0f,
                      true,
                      true,
                      {climate::CLIMATE_FAN_AUTO,
                       climate::CLIMATE_FAN_LOW,
                       climate::CLIMATE_FAN_MEDIUM,
                       climate::CLIMATE_FAN_HIGH},
                      {climate::CLIMATE_SWING_OFF,
                       climate::CLIMATE_SWING_VERTICAL})
            {
                this->set_supported_custom_presets({"Timer 30m",
                                                    "Timer 1h",
                                                    "Timer 1.5h",
                                                    "Timer 2h",
                                                    "Timer Off"});
            }

            void set_model(const Model model);
            void setup() override;

        protected:
            void control(const climate::ClimateCall &call) override;
            void transmit_state() override;
            void send() override;
            void apply_state() override;

        private:
            void apply_timer();
            void send_timer();

            bool timer_changed_ = false;

            IRSharpAc ac_ = IRSharpAc(255);
        };

    } // namespace sharp
} // namespace esphome
