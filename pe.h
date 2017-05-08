#ifndef PE_H
#define PE_H

#include "router.h"

SC_MODULE(PE_base)
{
        sc_in<bool> clock; // port for the global clock

        sc_in<packet> data_in; // from router
        sc_out<packet> data_out; // to router

        void main() // specify the functionality of PE per clock cycle
        {
                read_input();
                execute();
                write_output();
        }

        SC_CTOR(PE_base)
                : x_(-1), y_(-1)
        {
                SC_METHOD(main);
                sensitive << clock.pos();
        }

        // use this function to set the coordinates of the PE
        void set_xy(int x, int y);

        virtual ~PE_base() {}

protected:
        std::list<packet> out_queue_; // output queue
        packet packet_in_; // incoming packet from the router

        int x_, y_; // location of the PE

        virtual void read_input(); // read a packet from the the router
        virtual void execute() = 0; // abstraction of computations
        virtual void write_output(); // // send a packet to the router
}; // PE_base

// for PI and PO
class PE_IO : public PE_base
{
public:
        PE_IO(const char *name) : PE_base(name) {}

protected:
        void execute();

        void fire_PI();
        void fire_PO();
}; // class PE_IO

// for P1
class PE_inc : public PE_base
{
public:
        PE_inc(const char *name) : PE_base(name) {}

protected:
        void execute();

        void fire();
        void sendPkt(double, double, int, int, int, int);
};

#endif // PE_H
