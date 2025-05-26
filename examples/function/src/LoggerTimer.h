#include "Particle.h"

typedef std::function<void(void* p)> TimerCallback;

class TimerCallbackWrapper {
    private:
        TimerCallback m_cb = nullptr;
        void* m_cb_ctx = nullptr; 
    public:
        TimerCallbackWrapper(void* cb_ctx) : m_cb_ctx(cb_ctx){};

        void* ctx() { return m_cb_ctx; }

        inline TimerCallbackWrapper* operator=(TimerCallback cb){
            m_cb = cb;
            return this;
        }

        void emit(){
            if (m_cb) m_cb(m_cb_ctx);
        }
};