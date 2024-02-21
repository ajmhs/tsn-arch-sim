# TSN Simulator based on Docker containers 

The intention is to simulate a distributed application using RTI Connext whose DataWriters and DataReader communicate via specific VLANs leaving discovery and meta-traffic sent and received over the non-VLAN network


## Docker images
The first thing to do is to configure a docker image which will represent a node in the application architecture. There is a docker file in the `docker` subdirectory. To create the docker image, first copy the following files to this directory:

`rti_connext_dds-7.2.0-pro-host-x64Linux.run`  
`rti_connext_dds-7.2.0-pro-target-x64Linux4gcc7.3.0.rtipkg`  
`rti_license.dat`

(symlinking doesn't work, see Docker docs as to why...)

Adjust the line in the Dockerfile-7.2.0 that reads

`ENV TZ=Europe/Madrid`  
to set the correct timezone then run:

`docker build -t connext-7.2 -f Dockerfile-7.2.0 .`

in that directory to create the image.


## Networks
The docker images will be using bridged networking, and the default network interfaces will be those used for discovery and meta-traffic. In addition, there will be two networks added to simulate VLANs. To create these docker networks, run

```bash
docker network create --subnet=192.168.1.0/24 tsnnet1
docker network create --subnet=192.168.2.0/24 tsnnet2
```

On the development system the default docker bridged network creates a network at `172.17.0.0/16`. This provides two simulated VLANs and a "default" network configuration.


## Application configuration
The application will consist of three nodes based on the ShapeTypeExtended type used by ShapesDemo. An Orange sample will represent video command data, and Cyan samples will be used to represent effector command data. The samples differ in colour to permit easy visualisation in WireShark.

### Surgeon Console
discovery/meta-traffic: `172.17.0.3`  
Console/Effector VLAN (tsnnet1): `192.168.1.11`  
Console/Video VLAN (tsnnet2): `192.168.2.11`  

The "Surgeon Console" has two data writers, one to send commands to the "Video Server" (Orange) and one to send commands to the "Effector Server" (Cyan). 

### Video Server
discovery/meta-traffic: `172.17.0.4`  
Console/Video VLAN (tsnnet2): `192.168.2.12`  

The DataReader in the "Video Server" will receive samples via the tsnnet2 network.

### Effector Server
discovery/meta-traffic: `172.17.0.5`  
Console/Effector VLAN (tsnnet1): `192.168.1.13`  

The DataReader in the "Effector Server" will receive samples via the tsnnet1 network.

## Starting the docker containers
The default configuration used for the development of this simulation is to share the display with an X server to allow ShapesDemo to be executed from within the container and rendered on the host's display. 

If this isn't required, remove the 
`-e DISPLAY`, `-v $XAUTHORITY:/root/.Xauthority` and `-v /tmp/.X11-unix:/tmp/.X11-unix` parameters from the provided docker run commands.

### Surgeon Console
In a terminal window run:  
```bash
docker run --rm -it -e DISPLAY --privileged --hostname surgeon_console --name surgeon_console -v $XAUTHORITY:/root/.Xauthority -v /tmp/.X11-unix:/tmp/.X11-unix connext-7.2:latest bash
```

In a separate terminal window run the following commands to connect the "Surgeon Console" container to the tsnnet1 and tsnnet2 networks:

```bash
docker network connect tsnnet1 --ip 192.168.1.11 surgeon_console
docker network connect tsnnet2 --ip 192.168.2.11 surgeon_console
```

Run `ip a` in the "Surgeon Console" terminal to verify the network configuration:  

```bash
root@surgeon_console:~# ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
108: eth0@if109: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:ac:11:00:03 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 172.17.0.3/16 brd 172.17.255.255 scope global eth0
       valid_lft forever preferred_lft forever
110: eth1@if111: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:c0:a8:01:0b brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 192.168.1.11/24 brd 192.168.1.255 scope global eth1
       valid_lft forever preferred_lft forever
112: eth2@if113: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:c0:a8:02:0b brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 192.168.2.11/24 brd 192.168.2.255 scope global eth2
       valid_lft forever preferred_lft forever
```

### Video Server
In a new terminal window run:  
```bash
docker run --rm -it -e DISPLAY --privileged --hostname video_server --name video_server -v $XAUTHORITY:/root/.Xauthority -v /tmp/.X11-unix:/tmp/.X11-unix connext-7.2:latest bash
```

In a separate terminal window run the following command to connect the "Video Server" container to the tsnnet2 network:  

```bash
docker network connect tsnnet2 --ip 192.168.2.12 video_server
```

Run `ip a` in the "Video Server" terminal to verify the network configuration:  

```bash
root@video_server:~# ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
114: eth0@if115: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:ac:11:00:04 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 172.17.0.4/16 brd 172.17.255.255 scope global eth0
       valid_lft forever preferred_lft forever
116: eth1@if117: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:c0:a8:02:0c brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 192.168.2.12/24 brd 192.168.2.255 scope global eth1
       valid_lft forever preferred_lft forever
```       

### Effector Server
In a new terminal window run:  
```bash
docker run --rm -it -e DISPLAY --privileged --hostname effector_server --name effector_server -v $XAUTHORITY:/root/.Xauthority -v /tmp/.X11-unix:/tmp/.X11-unix connext-7.2:latest bash
```

In a separate terminal window run the following command to connect the "Effector Server" container to the tsnnet2 network:  

```bash
docker network connect tsnnet1 --ip 192.168.1.13 effector_server
```
Run `ip a` in the "Video Server" terminal to verify the network configuration:  

```bash
root@effector_server:~# ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
118: eth0@if119: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:ac:11:00:05 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 172.17.0.5/16 brd 172.17.255.255 scope global eth0
       valid_lft forever preferred_lft forever
120: eth1@if121: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:c0:a8:01:0d brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 192.168.1.13/24 brd 192.168.1.255 scope global eth1
       valid_lft forever preferred_lft forever
```

## Building the applications
From the `source/c++11` directory run 
```bash
make -f makefile_x64Linux4gcc7.3.0
```
and if the host configuration is correct, the three applications will be built. To simplify distribution of the applications and the associated Quality of Service profile files to the three docker containers, a bash script called `distribute.sh` has been provided.

Run the distribute script and then the three applications should be ready to be executed from within the docker containers.

## Usage
The three applications have the same usage:
```bash
Usage:
    -d, --domain       <int>   Domain ID this application will
                               subscribe in.  
                               Default: 0
    -s, --sample_count <int>   Number of samples to receive before
                               cleanly shutting down. 
                               Default: infinite
    -v, --verbosity    <int>   How much debugging output to show.
                               Range: 0-3 
                               Default: 1
```

## Quality of Service

The three applications have distinct QoS profiles, but they all follow a common theme. 
The base `tsn_profile` sets up and configures UDP transports which are given aliases defined as stdnet, tsnnet1 and tsnnet2. 
Only all three are defined in the "Surgeon Console" QoS, the other applications define only those that are used in their specific cases.

The transport_builtin QoS is set to MASK_NONE, meaning no builtin transports are enabled which ensures the traffic only goes through those registered. 

Discovery is configured to use the transport defined with the alias "stdnet", which is the default docker bridge network.
The initial peers are configured, as in a TSN scenario using VLANs it would be expected to know the addresses, and 
accept_unknown_peers is set to false.

### Surgeon Console

In the "Surgeon Console" QoS, two additional profiles are defined, both deriving from `tsn_profile`.
The first, `video_profile` configures the DataWriter for the `video_control` topic to use the transport with the `tsnnet2` alias,
while the second `effector_profile` configures the DataWriter for the `effector_control` topic to use the transport with the `tsnnet1` alias.

### Video Server

In the "Video Server" QoS, the discovery configuration differs in that only the "Surgeon Console" node is defined in the peer list.
The transport configuration also differs, as the transport with the alias `stdnet` is using the address docker assigned to the default bridge network (172.17.0.4), and the `tsnnet1` transport is absent as it is not used by the "Video Server".

In the `video_profile` QoS profile, the DataReader is configured to use the `tsnnet2` transport, and additionally the unicast endpoint is defined as using the `tsnnet2` transport and the specific receive port (2345). (This port can be anything as long as it doesn't conflict with those defined for builtin transports)

### Effector Server
In the "Effector Server" QoS, the discovery configuration again differs from that of the "Surgeon Console" in the discovery configuration.
The transport configuration again differs, as the transport with the alias `stdnet` is using the address docker assigned to the default bridge network (172.17.0.5), and the `tsnnet2` transport is absent as it is not used by the "Effector Server".

In the `effector_profile` QoS profile, the DataReader is configured to use the `tsnnet1` transport, and additionally the unicast endpoint is defined as using the `tsnnet1` transport and the specific receive port (1234).
