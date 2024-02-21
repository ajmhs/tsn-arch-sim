/*
* (c) Copyright, Real-Time Innovations, 2020.  All rights reserved.
* RTI grants Licensee a license to use, modify, compile, and create derivative
* works of the software solely for use with RTI Connext DDS. Licensee may
* redistribute copies of the software provided that all such copies are subject
* to this license. The software is provided "as is", with no warranty of any
* type, including any warranty for fitness for any purpose. RTI is under no
* obligation to maintain or support the software. RTI shall not be liable for
* any incidental or consequential damages arising out of the use or inability
* to use the software.
*/

#include <iostream>

#include <dds/pub/ddspub.hpp>
#include <rti/util/util.hpp>      // for sleep()
#include <rti/config/Logger.hpp>  // for logging
// alternatively, to include all the standard APIs:
//  <dds/dds.hpp>
// or to include both the standard APIs and extensions:
//  <rti/rti.hpp>
//
// For more information about the headers and namespaces, see:
//    https://community.rti.com/static/documentation/connext-dds/7.2.0/doc/api/connext_dds/api_cpp2/group__DDSNamespaceModule.html
// For information on how to use extensions, see:
//    https://community.rti.com/static/documentation/connext-dds/7.2.0/doc/api/connext_dds/api_cpp2/group__DDSCpp2Conventions.html

#include "application.hpp"  // for command line parsing and ctrl-c
#include "shapes.hpp"
#include <cmath>


void run_publisher_application(unsigned int domain_id, unsigned int sample_count)
{
    // DDS objects behave like shared pointers or value types
    // (see https://community.rti.com/best-practices/use-modern-c-types-correctly)

    //dds::core::QosProvider qos_provider("surgeon_qos_profiles.xml");
    dds::core::QosProvider qos_provider = dds::core::QosProvider::Default();

    // Start communicating in a domain, usually one participant per application
    dds::domain::DomainParticipant participant(domain_id);

    // Create a Topic with a name and a datatype
    dds::topic::Topic< ::ShapeTypeExtended> video_topic(participant, "video_control");
    dds::topic::Topic< ::ShapeTypeExtended> effector_topic(participant, "effector_control");

    // Create a Publisher
    dds::pub::Publisher publisher(participant);

    // Create a DataWriter with specific QoS
    dds::pub::DataWriter< ::ShapeTypeExtended> video_writer(publisher, video_topic, qos_provider.datawriter_qos("tsn_library::video_profile"));
    dds::pub::DataWriter< ::ShapeTypeExtended> effector_writer(publisher, effector_topic, qos_provider.datawriter_qos("tsn_library::effector_profile"));
    
    ::ShapeTypeExtended data;

    const int left = 15, top = 15, right = 248, bottom = 278; // limits
    const int shape_size = 30;
    int x = left-shape_size, y = bottom - top / 2;
    const float AMPLITUDE = 100.0f;
    const float FREQUENCY = 0.0475f;
    
    data.shapesize(shape_size);
    data.fillKind(ShapeFillKind::SOLID_FILL);
    
    // Main loop, write data
    unsigned int samples_written = 0;
    for (; !application::shutdown_requested && samples_written < sample_count; ++samples_written) {

        if (++x > right)
          x = left-shape_size;

        y = (int)(bottom - top) / 2 + AMPLITUDE * std::sin(FREQUENCY * x);
        
        data.x(x);
        data.y(y);

        std::cout << "Writing a video (ORANGE) and effector (CYAN) control sample, count: " << samples_written << std::endl;
        
        data.color("ORANGE");
        video_writer.write(data);
        data.color("CYAN");
        effector_writer.write(data);

        rti::util::sleep(dds::core::Duration(1));
    }
}

int main(int argc, char *argv[])
{

    using namespace application;

    // Parse arguments and handle control-C
    auto arguments = parse_arguments(argc, argv);
    if (arguments.parse_result == ParseReturn::exit) {
        return EXIT_SUCCESS;
    } else if (arguments.parse_result == ParseReturn::failure) {
        return EXIT_FAILURE;
    }
    setup_signal_handlers();

    // Sets Connext verbosity to help debugging
    rti::config::Logger::instance().verbosity(arguments.verbosity);

    try {
        run_publisher_application(arguments.domain_id, arguments.sample_count);
    } catch (const std::exception& ex) {
        // This will catch DDS exceptions
        std::cerr << "Exception in run_publisher_application(): " << ex.what()
        << std::endl;
        return EXIT_FAILURE;
    }

    // Releases the memory used by the participant factory.  Optional at
    // application exit
    dds::domain::DomainParticipant::finalize_participant_factory();

    return EXIT_SUCCESS;
}
