# Create the Docker image from RTI Connext 7.2

Copy the following files to this directory:

`rti_connext_dds-7.2.0-pro-host-x64Linux.run`  
`rti_connext_dds-7.2.0-pro-target-x64Linux4gcc7.3.0.rtipkg`  
`rti_license.dat`

(symlinking doesn't work, see Docker docs as to why...)

Adjust the line in the Dockerfile-7.2.0 that reads

`ENV TZ=Europe/Madrid`  
to set the correct timezone then run:

`docker build -t connext-7.2 -f Dockerfile-7.2.0 .`

to create the image.