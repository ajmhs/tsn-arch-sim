#!/bin/bash

# Script to push the executables and QoS profiles to the docker containers

docker cp objs/x64Linux4gcc7.3.0/surgeon_console surgeon_console:/root/
docker cp objs/x64Linux4gcc7.3.0/video_server video_server:/root/
docker cp objs/x64Linux4gcc7.3.0/effector_server effector_server:/root/

docker cp surgeon_qos_profiles.xml surgeon_console:/root/USER_QOS_PROFILES.xml
docker cp video_qos_profiles.xml video_server:/root/USER_QOS_PROFILES.xml
docker cp effector_qos_profiles.xml effector_server:/root/USER_QOS_PROFILES.xml