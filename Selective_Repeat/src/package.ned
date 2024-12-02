package selective_repeat;

import ned.IdealChannel;


@license(LGPL);
//
// TODO documentation
//
network Network
{
    @display("bgb=480,245");
    submodules:
        coordinator: Coordinator {
            @display("p=225,80");
        }
        node: Node {
            @display("p=93,157");
        }
        node1: Node {
            @display("p=355,162");
        }
    connections:
        node.Port <--> IdealChannel <--> coordinator.ports++;
        node1.Port <--> IdealChannel <--> coordinator.ports++;
}

