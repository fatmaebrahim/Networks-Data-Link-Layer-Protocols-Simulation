package selective_repeat;

import ned.IdealChannel;


@license(LGPL);
//
// TODO documentation
//
network Network
{
    parameters:
        int WS = default(4);
        int SN = default(7);
        double TO = default(10.0); // timeout interval
        double PT = default(0.5); // sender’s and receivers’ processing time
        double TD = default(1.0); // channel’s transmission delay
        double ED = default(4.0); // channel’s error delay ED
        double DD = default(1.0); //channel’s duplication delay
        double LP = default(0.0); // frame loss probability
    submodules:
        coordinator: Coordinator {
            @display("p=225,80");
        }
        node0: Node {
            @display("p=83,162");
        }
        node1: Node {
            @display("p=355,162");
        }
    connections:
        node0.Port <--> IdealChannel <--> coordinator.ports++;
        node1.Port <--> IdealChannel <--> coordinator.ports++;

        node1.out --> IdealChannel --> node0.in;
        node0.out --> IdealChannel --> node1.in;
}

