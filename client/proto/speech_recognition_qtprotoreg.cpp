
#include "speech_recognition.qpb.h"

#include <QtProtobuf/qprotobufregistration.h>

namespace bite_im {
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarSpeechRecognitionReq(qRegisterProtobufType<SpeechRecognitionReq>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarSpeechRecognitionRsp(qRegisterProtobufType<SpeechRecognitionRsp>);
static bool RegisterSpeech_recognitionProtobufTypes = [](){ qRegisterProtobufTypes(); return true; }();
} // namespace bite_im

