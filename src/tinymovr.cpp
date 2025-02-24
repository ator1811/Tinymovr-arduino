
#include <codec.h>
#include <tinymovr.h>

const uint8_t ep_bits = 6;
#define RECV_DELAY_US (160.0f)

uint8_t get_device_info_ep_id = 0x01A;
uint8_t get_state_ep_id = 0x003;
uint8_t get_encoder_estimates_ep_id = 0x009;
uint8_t get_Iq_set_est_ep_id = 0x014;
uint8_t get_pos_setpoint_ep_id = 0x00C;
uint8_t get_vel_setpoint_ep_id = 0x00D;

uint8_t set_state_ep_id = 0x007;
uint8_t set_pos_setpoint_ep_id = 0x00C;
uint8_t set_vel_setpoint_ep_id = 0x00D;
uint8_t set_Iq_setpoint_ep_id = 0x00E;
uint8_t reset_ep_id = 0x016;
uint8_t set_motor_config_ep_id = 0x01F;
uint8_t set_motor_RL_ep_id = 0x028;

void Tinymovr::device_info(uint32_t *device_id, uint8_t *fw_major,
    uint8_t *fw_minor, uint8_t *fw_patch, uint8_t *temp
) {
    this->send(get_device_info_ep_id, this->_data, 0, true);
    if (this->recv(get_device_info_ep_id, this->_data, &(this->_dlc), RECV_DELAY_US)) 
    {
        read_le(device_id, this->_data);
        read_le(fw_major, this->_data + 4);
        read_le(fw_minor, this->_data + 5);
        read_le(fw_patch, this->_data + 6);
        read_le(temp, this->_data + 7);
    }
}

void Tinymovr::idle()
{
    this->set_state(0, 0);
}

void Tinymovr::calibrate()
{
    this->set_state(1, 0);
}

void Tinymovr::cl_control()
{
    this->set_state(2, 0);
}

void Tinymovr::position_control()
{
    this->set_state(2, 2);
}

void Tinymovr::velocity_control()
{
    this->set_state(2, 1);
}

void Tinymovr::current_control()
{
    this->set_state(2, 0);
}

void Tinymovr::get_state(uint8_t *state, uint8_t *mode)
{
    this->send(get_state_ep_id, this->_data, 0, true);
    if (this->recv(get_state_ep_id, this->_data, &(this->_dlc), RECV_DELAY_US)) 
    {
        read_le(state, this->_data + 1);
        read_le(mode, this->_data + 2);
    }
}

void Tinymovr::get_encoder_estimates(float *pos_estimate, float *vel_estimate)
{
    this->send(get_encoder_estimates_ep_id, this->_data, 0, true);
    if (this->recv(get_encoder_estimates_ep_id, this->_data, &(this->_dlc), RECV_DELAY_US)) 
    {
        read_le(pos_estimate, this->_data);
        read_le(vel_estimate, this->_data + 4);
    }
}

void Tinymovr::get_Iq_setpoint_estimate(float *Iq_set, float *Iq_est)
{
    this->send(get_Iq_set_est_ep_id, this->_data, 0, true);
    if (this->recv(get_Iq_set_est_ep_id, this->_data, &(this->_dlc), RECV_DELAY_US)) 
    {
        read_le(Iq_set, this->_data);
        read_le(Iq_est, this->_data + 4);
    }
}

void Tinymovr::get_pos_setpoint(float *pos_setpoint, float *vel_ff, float *Iq_ff)
{
    int16_t vel_ff_;
    int8_t Iq_ff_;
    this->send(get_pos_setpoint_ep_id, this->_data, 0, true);
    if (this->recv(get_pos_setpoint_ep_id, this->_data, &(this->_dlc), RECV_DELAY_US)) 
    {
        read_le(pos_setpoint, this->_data);
        read_le(&vel_ff_, this->_data + 4);
        read_le(&Iq_ff_, this->_data + 4);
        *vel_ff = (float)(vel_ff_ * VEL_INT16_TO_FLOAT_FACTOR);
        *Iq_ff = (float)(Iq_ff_ * IQ_INT16_TO_FLOAT_FACTOR);
    }
}

void Tinymovr::get_vel_setpoint(float *vel_setpoint, float *Iq_ff)
{
    this->send(get_vel_setpoint_ep_id, this->_data, 0, true);
    if (this->recv(get_vel_setpoint_ep_id, this->_data, &(this->_dlc), RECV_DELAY_US)) 
    {
        read_le(vel_setpoint, this->_data);
        read_le(Iq_ff, this->_data + 4);
    }
}

void Tinymovr::set_state(uint8_t state, uint8_t mode)
{
    write_le(state, this->_data);
    write_le(mode, this->_data + 1);
    this->send(set_state_ep_id, this->_data, 2, false);
}

void Tinymovr::set_pos_setpoint(float pos_setpoint, float vel_ff, float Iq_cc)
{
    write_le(pos_setpoint, this->_data);
    write_le((int16_t)(vel_ff * VEL_FLOAT_TO_INT16_FACTOR), this->_data + 4);
    write_le((int16_t)(Iq_cc * IQ_FLOAT_TO_INT16_FACTOR), this->_data + 6);
    this->send(set_pos_setpoint_ep_id, this->_data, 8, false);
}

void Tinymovr::set_vel_setpoint(float vel_setpoint, float Iq_ff)
{
    write_le(vel_setpoint, this->_data);
    write_le(Iq_ff, this->_data + 4);
    this->send(set_vel_setpoint_ep_id, this->_data, 8, false);
}

void Tinymovr::set_Iq_setpoint(float Iq_setpoint)
{
    write_le(Iq_setpoint, this->_data);
    this->send(set_Iq_setpoint_ep_id, this->_data, 4, false);
}

void Tinymovr::reset()
{
    this->send(reset_ep_id, this->_data, 0, false);
}

void Tinymovr::send(uint8_t cmd_id, uint8_t *data, uint8_t data_size, bool rtr)
{
    const uint8_t arb_id = this->get_arbitration_id(cmd_id);
    this->send_cb(arb_id, data, data_size, rtr);
}

bool Tinymovr::recv(uint8_t cmd_id, uint8_t *data, uint8_t *data_size, uint16_t delay_us)
{
    // A delay of a few 100s of us needs to be inserted
    // to ensure the response has been transmitted.
    // TODO: Better handle this using an interrupt.
    if (delay_us > 0)
    {
        delayMicroseconds(delay_us);
    }
    const uint8_t arb_id = this->get_arbitration_id(cmd_id);
    return this->recv_cb(arb_id, data, data_size);
}

uint8_t Tinymovr::get_arbitration_id(uint8_t cmd_id)
{
    return this->can_node_id << ep_bits | cmd_id;
}

void Tinymovr::set_motor_config(uint8_t flags, uint8_t pole_pairs, float I_cal)
{
    write_le(flags, this->_data);
    write_le(pole_pairs, this->_data + 1);
    write_le(I_cal, this->_data + 2);
    this->send(set_motor_config_ep_id, this->_data, 8, false);
}

void Tinymovr::set_motor_RL(float R, float L)
{
    write_le(R, this->_data);
    write_le(L, this->_data + 4);
    this->send(set_motor_RL_ep_id, this->_data, 8, false);
}
