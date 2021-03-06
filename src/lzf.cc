/* node-lzf (C) 2015 newstein newstein33@gmail.com  */

#include <node_buffer.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#endif

#include "nan.h"

#include "lzf/lzf.h"


using namespace v8;
using namespace node;


// Handle<Value> ThrowNodeError(const char* what = NULL) {
//     return Nan::ThrowError(Exception::Error(Nan::New<String>(what))).ToLocalChecked();
// }
NAN_METHOD(compress) {
    if (info.Length() < 1 || !Buffer::HasInstance(info[0])) {
        return Nan::ThrowError("First argument must be a Buffer");
    }
    Nan::HandleScope scope;

    Local<Object> bufferIn = info[0]->ToObject();
    size_t bytesIn         = Buffer::Length(bufferIn);
    char * dataPointer     = Buffer::Data(bufferIn);
    size_t bytesCompressed = bytesIn + 100;
    char * bufferOut       = (char*) malloc(bytesCompressed);

    if (!bufferOut) {
        return Nan::ThrowError("LZF malloc failed!");
    }

    unsigned result = lzf_compress(dataPointer, bytesIn, bufferOut, bytesCompressed);

    if (!result) {
        free(bufferOut);
        return Nan::ThrowError("Compression failed, probably too small buffer");
    }

    Local<Object> BufferOut = Nan::NewBuffer(bufferOut, result).ToLocalChecked();
    free(bufferOut);

    info.GetReturnValue().Set(BufferOut);
}


NAN_METHOD(decompress) {
    if (info.Length() < 1 || !Buffer::HasInstance(info[0])) {
        return Nan::ThrowError("First argument must be a Buffer");
    }

    Local<Object> bufferIn = info[0]->ToObject();

    size_t bytesUncompressed = 999 * 1024 * 1024; // it's about max size that V8 supports

    if (info.Length() > 1 && info[1]->IsNumber()) { // accept dest buffer size
        bytesUncompressed = info[1]->Uint32Value();
    }


    char * bufferOut = (char*) malloc(bytesUncompressed);
    if (!bufferOut) {
        return Nan::ThrowError("LZF malloc failed!");
    }

    unsigned result = lzf_decompress(Buffer::Data(bufferIn), Buffer::Length(bufferIn), bufferOut, bytesUncompressed);

    if (!result) {
        return Nan::ThrowError("Unrompression failed, probably too small buffer");
    }

    Local<Object> BufferOut = Nan::NewBuffer(bufferOut, result).ToLocalChecked();

    free(bufferOut);

    Nan::HandleScope scope;
    info.GetReturnValue().Set(BufferOut);
}

extern "C" void
init (Handle<Object> target) {
    Nan::SetMethod(target, "compress", compress);
    Nan::SetMethod(target, "decompress", decompress);
}

NODE_MODULE(lzf, init)
