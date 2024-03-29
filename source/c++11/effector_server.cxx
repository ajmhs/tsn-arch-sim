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

#include <algorithm>
#include <iostream>

#include <dds/sub/ddssub.hpp>
#include <dds/core/ddscore.hpp>
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

#include "shapes.hpp"
#include "application.hpp"  // for command line parsing and ctrl-c

void process_data(dds::sub::DataReader< ::ShapeTypeExtended> reader, unsigned int& samples_read)
{
    // Take all samples
    //int count = 0;
    dds::sub::LoanedSamples< ::ShapeTypeExtended> samples = reader.take();
    for (auto sample : samples) {
        if (sample.info().valid()) {            
            std::cout << "Read a effector control sample (\"" << sample.data().color() << "\"), count: " << ++samples_read << std::endl;
        } else {
            std::cout << "Instance state changed to "
            << sample.info().state().instance_state() << std::endl;
        }
    }
} // The LoanedSamples destructor returns the loan

void run_subscriber_application(unsigned int domain_id, unsigned int sample_count)
{
    // DDS objects behave like shared pointers or value types
    // (see https://community.rti.com/best-practices/use-modern-c-types-correctly)

    //dds::core::QosProvider qos_provider("effector_qos_profiles.xml");
    dds::core::QosProvider qos_provider = dds::core::QosProvider::Default();

    // Start communicating in a domain, usually one participant per application
    dds::domain::DomainParticipant participant(domain_id);

    // Create a Topic with a name and a datatype
    dds::topic::Topic< ::ShapeTypeExtended> topic(participant, "effector_control");

    // Create a Subscriber and DataReader with specific Qos
    dds::sub::Subscriber subscriber(participant);
    dds::sub::DataReader< ::ShapeTypeExtended> reader(subscriber, topic, qos_provider.datareader_qos("tsn_library::effector_profile"));

    // Create a ReadCondition for any data received on this reader and set a
    // handler to process the data
    unsigned int samples_read = 0;
    dds::sub::cond::ReadCondition read_condition(
        reader,
        dds::sub::status::DataState::any(),
        [reader, &samples_read]() { process_data(reader, samples_read); });

    // WaitSet will be woken when the attached condition is triggered
    dds::core::cond::WaitSet waitset;
    waitset += read_condition;

    std::cout << "::Effector control server waiting for connection" << std::endl;

    while (!application::shutdown_requested && samples_read < sample_count) {
        
        // Run the handlers of the active conditions. Wait for up to 1 second.
        waitset.dispatch(dds::core::Duration(1));
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
        run_subscriber_application(arguments.domain_id, arguments.sample_count);
    } catch (const std::exception& ex) {
        // This will catch DDS exceptions
        std::cerr << "Exception in run_subscriber_application(): " << ex.what()
        << std::endl;
        return EXIT_FAILURE;
    }

    // Releases the memory used by the participant factory.  Optional at
    // application exit
    dds::domain::DomainParticipant::finalize_participant_factory();

    return EXIT_SUCCESS;
}
