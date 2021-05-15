//
// Created by OnikenX on 5/15/2021.
//

#include <utils.hpp>

Semaforos *Semaforos::create() {
    static Semaforos s;
    if (s.firsttime) {
        s.semaforo_read_control_aviao =
                CreateSemaphore(NULL, 0, CIRCULAR_BUFFERS_SIZE,SEMAFORO_READ_CONTROL_AVIAO);
        s.semaforo_write_control_aviao =
                CreateSemaphore(NULL, CIRCULAR_BUFFERS_SIZE, CIRCULAR_BUFFERS_SIZE,SEMAFORO_WRITE_CONTROL_AVIAO);
        s.semaforo_read_aviao_control =
                CreateSemaphore(NULL, 0, CIRCULAR_BUFFERS_SIZE,SEMAFORO_READ_AVIAO_CONTROL);
        s.semaforo_write_aviao_control =
                CreateSemaphore(NULL, CIRCULAR_BUFFERS_SIZE, CIRCULAR_BUFFERS_SIZE,SEMAFORO_WRITE_AVIAO_CONTROL);
        s.firsttime = false;
        if (!s.semaforo_read_control_aviao || !s.semaforo_write_control_aviao ||
        !s.semaforo_write_aviao_control || !s.semaforo_read_aviao_control) {
            s.closeall();
            s.erros = true;
            return nullptr;
        }
        return &s;
    } else {
        if (s.erros)
            return nullptr;
        else
            return &s;
    }
}

void Semaforos::closeall() {
    if (this->semaforo_read_control_aviao)
        CloseHandle(this->semaforo_read_control_aviao);
    if (this->semaforo_write_control_aviao)
        CloseHandle(this->semaforo_write_control_aviao);
    if (this->semaforo_read_aviao_control)
        CloseHandle(this->semaforo_read_aviao_control);
    if (this->semaforo_write_aviao_control) {
        CloseHandle(this->semaforo_write_aviao_control);
    }
    this->semaforo_read_control_aviao = this->semaforo_write_control_aviao = this->semaforo_read_aviao_control = this->semaforo_write_aviao_control = nullptr;
}

Semaforos::Semaforos() : semaforo_read_control_aviao(nullptr), semaforo_write_control_aviao(nullptr),
                         semaforo_read_aviao_control(nullptr), semaforo_write_aviao_control(nullptr), firsttime(true),
                         erros(false) {}

Semaforos::~Semaforos() {
    closeall();
}
