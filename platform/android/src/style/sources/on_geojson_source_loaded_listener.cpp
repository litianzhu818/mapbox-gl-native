#include "on_geojson_source_loaded_listener.hpp"
#include "../../java/util.hpp"

namespace mbgl {
namespace android {

    void OnGeoJsonSourceLoadedListener::registerNative(jni::JNIEnv& env) {
        // Lookup the class
        javaClass = *jni::Class<OnGeoJsonSourceLoadedListener>::Find(env).NewGlobalRef(env).release();
    }

    void OnGeoJsonSourceLoadedListener::notifySourceLoaded(jni::JNIEnv &env, jni::Object<OnGeoJsonSourceLoadedListener> jListener) {
        auto method = OnGeoJsonSourceLoadedListener::javaClass.GetMethod<void ()>(env, "onGeoJsonSourceLoaded");
        jListener.Call(env, method);
    }

    jni::Class<OnGeoJsonSourceLoadedListener> OnGeoJsonSourceLoadedListener::javaClass;

} // namespace android
} // namespace mbgl