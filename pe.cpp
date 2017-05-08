#include "pe.h"
#include "router.h"


int cnt=0;
int pchk=0;
int out=1;
int nxt=1;
extern int c;
int clkc=0, in=0, op=0;
int num = 0, input[100000], output[100];
int stg1[100]={1}, stg2[100]={1}, stg3[100]={1}, stg4[100]={1};



packet packet_in_2[10]={-1, -1, -1, -1};

void PE_base::set_xy(int x, int y)
{
        assert((x_ == -1) && (y_ == -1)); // set once only
        assert((x != -1) && (y != -1)); // must use a legal location

        x_ = x;
        y_ = y;
}

void PE_base::read_input()
{
        packet_in_ = data_in.read();
}

void PE_base::write_output()
{
        if (out_queue_.empty())
        {
                data_out.write(packet());
        }
        else
        {
                data_out.write(out_queue_.front());
                out_queue_.pop_front();
        }
}
void PE_IO::execute()
{
        // decide if we are going to fire PI
        int r = rand()%100;
        if (r<50)          // If we use while instead of if then it will be used to fire multiple inputs over different cycles
        {
                fire_PI();
                r = rand()%100;
        }

        // fire PO if the incoming packet is valid
        if ((packet_in_.src_x != -1)
                && (packet_in_.src_y != -1))
                fire_PO();

        
}

void PE_IO::fire_PI()
{
        int P1_x=0, P1_y=0, value[8]={1,0,2,0,3,0,4,0};//{1,2,3,4,0,0,0,0};

        for (cnt=0; cnt<8; cnt++)
        {
                if(cnt == 4 || cnt == 0) // As per the equations mentioned in the project 2 instructions manual
                {
                        P1_x = 0; P1_y = 0;
                }
                else if(cnt == 2 || cnt == 6) // As per the equations mentioned in the project 2 instructions manual
                {
                        P1_x = 0; P1_y = 1;
                }
                else if(cnt == 1 || cnt == 5) //  As per the equations mentioned in the project 2 instructions manual
                {
                        P1_x = 2; P1_y = 2;
                }
                else if(cnt == 3 || cnt == 7) //  As per the equations mentioned in the project 2 instructions manual
                {
                        P1_x = 1; P1_y = 2;
                }
                packet p(x_, y_, P1_x, P1_y, nxt, num, value[cnt], 0);

                printf("PI: send %f + %f  to (%d,%d) -- flag:%d, pkt:%d\n",
                p.token, p.tokeny, p.dest_x, p.dest_y, p.flag, p.pkt);

                input[nxt]=c;
                out_queue_.push_back(p);
                num++;
        }

        if (cnt > 7)                  // This part is for the packet calculation
        {
                num=0;
                cnt = 0;
                nxt = nxt+1;
        }

}

void PE_IO::fire_PO()
{
        assert((packet_in_.src_x != -1)
                && (packet_in_.src_y != -1));

        printf("PO: received X[%d] = %f + %f from (%d,%d)\n", packet_in_.pkt,
                packet_in_.token, packet_in_.tokeny, packet_in_.src_x,  packet_in_.src_y);

        op = op+1;

        if ((stg1[out]>3 && stg2[out]>3 && stg3[out]>3 && stg4[out]>3) && op==8)
        {
                output[out]=c;
                op=1;
                clkc = c;
                in = nxt-1;
                out = out+1;
        }

}

void PE_inc::execute()
{
        // fire the actor if the incoming packet is valid
        if ((packet_in_.src_x != -1)
                && (packet_in_.src_y != -1))
                fire();
}

void PE_inc::fire()
{
        assert((packet_in_.src_x != -1)
                && (packet_in_.src_y != -1));

        int i1=0, j1=0, k1=0;
        int i2=0, j2=0, k2=0;
        double rvalue1=0, ivalue1=0, rvalue2=0, ivalue2=0;
        double real_om = 1/1.414, img_om = -(1/1.414);
        double real_om2 = 0, img_om2 = -1;
        double real_om3 = -(real_om), img_om3 = img_om;

        if ((packet_in_2[x_*3+y_].src_x == -1) && (packet_in_2[x_*3+y_].src_y == -1))
        {
                packet_in_2[x_*3+y_] = packet_in_;
                pchk = 1;
        }

        if (((packet_in_2[x_*3+y_].src_x != -1) && (packet_in_2[x_*3+y_].src_y != -1)) && pchk==0  && packet_in_.flag == packet_in_2[x_*3+y_].flag)
        {
                if((packet_in_2[x_*3+y_].pkt) > (packet_in_.pkt))
                {
                        packet_in_2[10] = packet_in_;
                        packet_in_ = packet_in_2[x_*3+y_];
                        packet_in_2[x_*3+y_] = packet_in_2[10];
                        packet_in_2[10].src_x = -1;
                }
                switch(x_*3+y_)
                {
                case 0:
                        stg1[packet_in_.flag]++;
                        rvalue1 = packet_in_2[x_*3+y_].token + packet_in_.token;
                        ivalue1 = packet_in_2[x_*3+y_].tokeny + packet_in_.tokeny;
                        i1 = 2; j1 = 0; k1=0;

                        rvalue2 = packet_in_2[x_*3+y_].token - packet_in_.token;
                        ivalue2 = packet_in_2[x_*3+y_].tokeny - packet_in_.tokeny;
                        i2 = 0; j2 = 2; k2=1;
                        stg1[packet_in_.flag]++;
                        break;
                case 1:
                        stg2[packet_in_.flag]++;
                        rvalue1 = packet_in_2[x_*3+y_].token + packet_in_.token;
                        ivalue1 = packet_in_2[x_*3+y_].tokeny + packet_in_.tokeny;
                        i1 = 2; j1 = 0; k1=2;

                        rvalue2 = packet_in_2[x_*3+y_].token - packet_in_.token;
                        ivalue2 = packet_in_2[x_*3+y_].tokeny - packet_in_.tokeny;
                        i2 = 0; j2 = 2; k2=3;
                        stg2[packet_in_.flag]++;
                        break;
                case 2:
                        if(stg2[packet_in_.flag] == 2)
                        {
                                rvalue1 = packet_in_2[x_*3+y_].token + ((packet_in_.token*real_om2)-(packet_in_.tokeny*img_om2));
                                ivalue1 = packet_in_2[x_*3+y_].tokeny + ((packet_in_.token*img_om2)+(packet_in_.tokeny*real_om2));
                                i1 = 0; j1= 2; k1=2;

                                rvalue2 = packet_in_2[x_*3+y_].token - ((packet_in_.token*real_om2)-(packet_in_.tokeny*img_om2));
                                ivalue2 = packet_in_2[x_*3+y_].tokeny - ((packet_in_.token*img_om2)+(packet_in_.tokeny*real_om2));
                                i2 = 1; j2 = 0; k2=3;
                                stg2[packet_in_.flag]++;
                        }
                        else if(stg2[packet_in_.flag] == 3)
                        {
                                rvalue1 = packet_in_2[x_*3+y_].token + ((packet_in_.token*real_om)-(packet_in_.tokeny*img_om));
                                ivalue1 = packet_in_2[x_*3+y_].tokeny + ((packet_in_.token*img_om)+(packet_in_.tokeny*real_om));
                                i1 = 1; j1 = 1; k1=1;

                                rvalue2 = packet_in_2[x_*3+y_].token - ((packet_in_.token*real_om)-(packet_in_.tokeny*img_om));
                                ivalue2 = packet_in_2[x_*3+y_].tokeny - ((packet_in_.token*img_om)+(packet_in_.tokeny*real_om));
                                i2 = 1; j2 = 1; k2=5;
                                stg2[packet_in_.flag]++;
                        }
                        break;
                case 3:
                        if(stg4[packet_in_.flag] == 2)
                        {
                                rvalue1 = packet_in_2[x_*3+y_].token + ((packet_in_.token*real_om2)-(packet_in_.tokeny*img_om2));
                                ivalue1 = packet_in_2[x_*3+y_].tokeny + ((packet_in_.token*img_om2)+(packet_in_.tokeny*real_om2));
                                i1=0; j1=2; k1=5;

                                rvalue2 = packet_in_2[x_*3+y_].token - ((packet_in_.token*real_om2)-(packet_in_.tokeny*img_om2));
                                ivalue2 = packet_in_2[x_*3+y_].tokeny - ((packet_in_.token*img_om2)+(packet_in_.tokeny*real_om2));
                                i2=1; j2=0; k2=7;
                                stg4[packet_in_.flag]++;
                        }
                        else if(stg4[packet_in_.flag] == 3)
                        {
                        rvalue1 = packet_in_2[x_*3+y_].token + ((packet_in_.token*real_om3)-(packet_in_.tokeny*img_om3));
                        ivalue1 = packet_in_2[x_*3+y_].tokeny + ((packet_in_.token*img_om3)+(packet_in_.tokeny*real_om3));
                        i1=1; j1=1; k1=3;

                        rvalue2 = packet_in_2[x_*3+y_].token - ((packet_in_.token*real_om3)-(packet_in_.tokeny*img_om3));
                        ivalue2 = packet_in_2[x_*3+y_].tokeny - ((packet_in_.token*img_om3)+(packet_in_.tokeny*real_om3));
                        i2=1; j2=1; k2=7;
                        stg4[packet_in_.flag]++;
                        }
                        break;
                case 5:
                        stg4[packet_in_.flag]++;
                        rvalue1 = packet_in_2[x_*3+y_].token + packet_in_.token;
                        ivalue1 = packet_in_2[x_*3+y_].tokeny + packet_in_.tokeny;
                        i1 = 2; j1 = 1; k1=6;

                        rvalue2 = packet_in_2[x_*3+y_].token - packet_in_.token;
                        ivalue2 = packet_in_2[x_*3+y_].tokeny - packet_in_.tokeny;
                        i2 = 1; j2 = 0; k2=7;
                        stg4[packet_in_.flag]++;
                        break;
                case 6:
                        if (stg1[packet_in_.flag] == 2)
                        {
                                rvalue1 = packet_in_2[x_*3+y_].token + packet_in_.token;
                                ivalue1 = packet_in_2[x_*3+y_].tokeny + packet_in_.tokeny;
                                i1=2; j1=0; k1=0;

                                rvalue2 = packet_in_2[x_*3+y_].token - packet_in_.token;
                                ivalue2 = packet_in_2[x_*3+y_].tokeny - packet_in_.tokeny;
                                i2=2; j2=1; k2=1;
                                stg1[packet_in_.flag]++;
                        }
                        else if(stg1[packet_in_.flag] == 3)
                        {
                                rvalue1 = packet_in_2[x_*3+y_].token + packet_in_.token;
                                ivalue1 = packet_in_2[x_*3+y_].tokeny + packet_in_.tokeny;
                                i1=1; j1=1; k1=0;

                                rvalue2 = packet_in_2[x_*3+y_].token - packet_in_.token;
                                ivalue2 = packet_in_2[x_*3+y_].tokeny - packet_in_.tokeny;
                                i2=1; j2=1; k2=4;
                                stg1[packet_in_.flag]++;
                        }
                        break;
                case 7:
                        if(stg3[packet_in_.flag] == 2)
                        {
                                rvalue1 = packet_in_2[x_*3+y_].token + packet_in_.token;
                                ivalue1 = packet_in_2[x_*3+y_].tokeny + packet_in_.tokeny;
                                i1=2; j1=0; k1=4;

                                rvalue2= packet_in_2[x_*3+y_].token - packet_in_.token;
                                ivalue2= packet_in_2[x_*3+y_].tokeny - packet_in_.tokeny;
                                i2=2; j2=1; k2=5;
                                stg3[packet_in_.flag]++;
                        }
                        else if(stg3[packet_in_.flag] == 3)
                        {
                                rvalue1 = packet_in_2[x_*3+y_].token + ((packet_in_.token*real_om2)-(packet_in_.tokeny*img_om2));
                                ivalue1 = packet_in_2[x_*3+y_].tokeny + ((packet_in_.token*img_om2)+(packet_in_.tokeny*real_om2));
                                i1=1; j1=1; k1=2;

                                rvalue2= packet_in_2[x_*3+y_].token - ((packet_in_.token*real_om2)-(packet_in_.tokeny*img_om2));
                                ivalue2 = packet_in_2[x_*3+y_].tokeny - ((packet_in_.token*img_om2)+(packet_in_.tokeny*real_om2));
                                i2=1; j2=1;k2=6;
                                stg3[packet_in_.flag]++;
                        }
                        break;
                case 8:
                        stg3[packet_in_.flag]++;
                        rvalue1 = packet_in_2[x_*3+y_].token + packet_in_.token;
                        ivalue1 = packet_in_2[x_*3+y_].tokeny + packet_in_.tokeny;
                        i1 = 2; j1 = 1; k1=4;

                        rvalue2= packet_in_2[x_*3+y_].token - packet_in_.token;
                        ivalue2= packet_in_2[x_*3+y_].tokeny - packet_in_.tokeny;
                        i2 = 1; j2 = 0; k2=5;
                        stg3[packet_in_.flag]++;
                        break;

                default:
                        break;


                }
                sendPkt(rvalue1, ivalue1, packet_in_.flag, k1, i1, j1);
                sendPkt(rvalue2, ivalue2, packet_in_.flag, k2, i2, j2);
                packet_in_2[x_*3+y_].src_x=-1;
                packet_in_2[x_*3+y_].src_y=-1;
                
        }
        else if(packet_in_.flag < packet_in_2[x_*3+y_].flag)
        {
                out_queue_.push_back(packet_in_2[x_*3+y_]);
                packet_in_2[x_*3+y_] = packet_in_;
        }
        else if(packet_in_.flag > packet_in_2[x_*3+y_].flag)
        {
                out_queue_.push_back(packet_in_);
        }

        pchk=0;
}
        
void PE_inc::sendPkt(double rvalue, double ivalue, int f, int k, int x, int y)
{

        packet p(x_, y_, x, y, f, k, rvalue, ivalue);

        printf("inc(%d,%d): send %f + %f to (%d,%d)\n",
                x_, y_, rvalue, ivalue, x, y);
        out_queue_.push_back(p);

}
