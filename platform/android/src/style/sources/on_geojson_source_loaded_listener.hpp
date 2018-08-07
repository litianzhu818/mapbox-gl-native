#pragma once

#include <mbgl/util/noncopyable.hpp>
#include <jni/jni.hpp>

namespace mbgl {
namespace android {
    class OnGeoJsonSourceLoadedListener : private mbgl::util::noncopyable {
    public:
        static constexpr auto Name() { return "com/mapbox/mapboxsdk/style/sources/OnGeoJsonSourceLoadedListener"; };

        static jni::Class<OnGeoJsonSourceLoadedListener> javaClass;

        static void notifySourceLoaded(jni::JNIEnv& env, jni::Object<OnGeoJsonSourceLoadedListener>);

        static void registerNative(jni::JNIEnv&);
    };


} // namespace android
} // namespace mbgl