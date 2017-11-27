#ifndef INPUT_RECEIVER_H
#define INPUT_RECEIVER_H

class InputReceiver : public IEventReceiver {

    public:

        struct SMouseState {
            core::position2di Position;
            bool LeftButtonDown;
            bool RightButtonDown;
            SMouseState() : LeftButtonDown(false) { }
        } MouseState;

        virtual bool OnEvent(const SEvent& event) {
            if (event.EventType == irr::EET_KEY_INPUT_EVENT) {
                KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
            }

            if (event.EventType == irr::EET_MOUSE_INPUT_EVENT) {

                switch(event.MouseInput.Event) {
                    case EMIE_LMOUSE_PRESSED_DOWN:
                        MouseState.LeftButtonDown = true;
                        break;

                    case EMIE_LMOUSE_LEFT_UP:
                        MouseState.LeftButtonDown = false;
                        break;

                    case EMIE_RMOUSE_PRESSED_DOWN:
                        MouseState.RightButtonDown = true;
                        break;

                    case EMIE_RMOUSE_LEFT_UP:
                        MouseState.RightButtonDown = false;
                        break;

                    case EMIE_MOUSE_MOVED:
                        MouseState.Position.X = event.MouseInput.X;
                        MouseState.Position.Y = event.MouseInput.Y;
                        break;

                    default:
                        break;
                }
            }

            return false;
        }

        virtual bool IsKeyDown(EKEY_CODE keyCode) const {
            return KeyIsDown[keyCode];
        }

        const SMouseState& GetMouseState(void) const {
            return MouseState;
        }

        InputReceiver() {
            for (u32 i = 0; i < KEY_KEY_CODES_COUNT; i++) {
                KeyIsDown[i] = false;
            }
        }

    private:

        bool KeyIsDown[KEY_KEY_CODES_COUNT];
};
#endif
