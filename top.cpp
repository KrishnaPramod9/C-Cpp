#include <time.h>
#include <systemc.h>

#include "router.h"
#include "pe.h"

extern int clkc, in;
extern int out, input[100000], output[100], nxt;
int c=0;

SC_MODULE(top)
{
public:
        enum {N = 3};

        router *routers[N][N]; // We use matrix notation since we have to implement 2-D (3*3) NoC Architecture
        PE_base *pes[3*N];

        sc_signal<packet> router_to_pe[N][N], pe_to_router[N][N];
        sc_signal<packet> router_to_router_east[N-1][N], router_to_router_west[N-1][N];
        sc_signal<packet> router_to_router_north[N][N-1], router_to_router_south[N][N-1];
        sc_signal<packet> terminal_loop_north[N], terminal_loop_south[N];
        sc_signal<packet> terminal_loop_east[N], terminal_loop_west[N];
        sc_signal<bool> clock;

        SC_CTOR(top)
        {
                create_pes();
                create_network();
        }

protected:
        void create_pes()
        {
                pes[0] = new PE_inc("P1");
                pes[0]->clock(clock);
                pes[0]->set_xy(0, 0);

                pes[1] = new PE_inc("P2");
                pes[1]->clock(clock);
                pes[1]->set_xy(1, 0);

                pes[2] = new PE_inc("P3");
                pes[2]->clock(clock);
                pes[2]->set_xy(2, 0);

                pes[3] = new PE_inc("P4");
                pes[3]->clock(clock);
                pes[3]->set_xy(0, 1);

                pes[4] = new PE_IO("PI/PO");
                pes[4]->clock(clock);
                pes[4]->set_xy(1, 1);

                pes[5] = new PE_inc("P5");
                pes[5]->clock(clock);
                pes[5]->set_xy(2, 1);

                pes[6] = new PE_inc("P6");
                pes[6]->clock(clock);
                pes[6]->set_xy(0, 2);

                pes[7] = new PE_inc("P7");
                pes[7]->clock(clock);
                pes[7]->set_xy(1, 2);

                pes[8] = new PE_inc("P8");
                pes[8]->clock(clock);
                pes[8]->set_xy(2, 2);

        }

        void create_network()
        {
                for (int j = 0; j < N; ++j)
                {
                        for (int i = 0; i < N; ++i)
                        {
                                char name[100];
                                sprintf(name, "router%d%d",i,j);

                                // router creation
                                routers[i][j] = new router(name);
                                routers[i][j]->set_xy(i,j);
                                routers[i][j]->clock(clock);

                                // router to north routers connection
                                if (j != 0)
                                {
                                        routers[i][j]->port_out[router::NORTH](
                                                router_to_router_north[i][j-1]);
                                        routers[i][j]->port_in[router::NORTH](
                                                router_to_router_south[i][j-1]);
                                }
                                else
                                {
                                        routers[i][j]->port_out[router::NORTH](
                                                terminal_loop_north[i]);
                                        routers[i][j]->port_in[router::NORTH](
                                                terminal_loop_north[i]);
                                }
                                // router to south routers connection
                                if (j!=N-1)
                                {
                                        routers[i][j]->port_out[router::SOUTH](
                                                router_to_router_south[i][j]);
                                        routers[i][j]->port_in[router::SOUTH](
                                                router_to_router_north[i][j]);
                                }
                                else 
                                {
                                        routers[i][j]->port_out[router::SOUTH](
                                                terminal_loop_south[i]);
                                        routers[i][j]->port_in[router::SOUTH](
                                                terminal_loop_south[i]);
                                }


                                // router to west routers connection
                                if (i != 0)
                                {
                                        routers[i][j]->port_out[router::WEST](
                                                router_to_router_west[i-1][j]);
                                        routers[i][j]->port_in[router::WEST](
                                                router_to_router_east[i-1][j]);
                                }
                                else 
                                {
                                        routers[i][j]->port_out[router::WEST](
                                                terminal_loop_west[j]);
                                        routers[i][j]->port_in[router::WEST](
                                                terminal_loop_west[j]);
                                }

                                if (i != N-1) // router to east routers connection
                                {
                                        routers[i][j]->port_out[router::EAST](
                                                router_to_router_east[i][j]);
                                        routers[i][j]->port_in[router::EAST](
                                                router_to_router_west[i][j]);
                                }
                                else 
                                {
                                        routers[i][j]->port_out[router::EAST](
                                                terminal_loop_east[j]);
                                        routers[i][j]->port_in[router::EAST](
                                                terminal_loop_east[j]);
                                }

                                // router to PE connection
                                routers[i][j]->port_out[router::PE](router_to_pe[i][j]);
                                routers[i][j]->port_in[router::PE](pe_to_router[i][j]);
                                pes[3*j+i]->data_in(router_to_pe[i][j]);
                                pes[3*j+i]->data_out(pe_to_router[i][j]);
                        }
                }
        }

};

int sc_main(int argc , char *argv[])
{
        srand(0);

        top top_module("top");

        int sum=0, lat[1000], min=1000;
        int total=5000;
        double thruput;

        printf("cycle  0 ================================\n");
        sc_start(0, SC_NS);

        for(int cycle = 1; cycle < total; cycle++)
        {

                printf("cycle %2d ================================\n", cycle);

                top_module.clock.write(1);
                sc_start(10, SC_NS);
                top_module.clock.write(0);
                sc_start(10, SC_NS);
                c=cycle;
        }

             printf("Generated outputs is: %d\n", out-1);
        for (int i=1; i<=out-1; i++)
        {
                lat[i]=output[i]-input[i];
                sum = sum +lat[i];
                if (lat[i]<min)
                        min =lat[i];
        }
        thruput=float(nxt-1)/(float)total;
        printf("Number of generated input sets is = %d\n", nxt-1);
        printf("Number of cycles is = %d\n", total);
        printf("Total Throughput obtained is = %f\n", thruput);
        printf("The minimum lat obtained is = %d\n", min);
        printf("The average lat obtained is = %d\n", sum/(out-1));

        return 0;
}
