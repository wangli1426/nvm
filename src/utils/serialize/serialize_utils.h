//
// Created by robert on 1/11/17.
//

#ifndef NVM_SERIALIZE_UTILS_H
#define NVM_SERIALIZE_UTILS_H
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

void register_derived_classes(boost::archive::text_iarchive &ia);

template<typename T>
T utils_deserialize(void* input_buffer, uint32_t size) {
    boost::iostreams::basic_array_source<char> device((char*)input_buffer,
                                                      size);
    boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(
            device);
    boost::archive::text_iarchive ia(s);

    T ret;
    ia >> ret;
    return ret;
}

template<typename T>
void utils_serialize(void* output_buffer, uint32_t size, T& t) {
    boost::iostreams::basic_array_sink<char> device((char*)output_buffer, size);
    boost::iostreams::stream<boost::iostreams::basic_array_sink<char> > s (device);
    boost::archive::text_oarchive oa(s);

    t >> oa;
}

#endif //NVM_SERIALIZE_UTILS_H
