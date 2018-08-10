#pragma once

#include "source.hpp"
#include <mbgl/style/sources/geojson_source.hpp>
#include "../../geojson/geometry.hpp"
#include "../../geojson/feature.hpp"
#include "../../geojson/feature_collection.hpp"
#include <jni/jni.hpp>

namespace mbgl {
namespace android {

using Callback = std::function<void (GeoJSON)>;

struct FeatureConverter {
    void convertJson(jni::String, ActorRef<Callback>);

    template <class JNIType>
    void convertObject(jni::Object<JNIType>, ActorRef<Callback>);
};

struct Update {
    std::shared_ptr<jni::jobject> object;
    std::unique_ptr<Actor<Callback>> callback;

    using Updater = std::function<void (ActorRef<FeatureConverter>)>;
    Updater updateFn;

    template <class JNIType>
    void makeUpdateFn();

    Update(std::shared_ptr<jni::jobject>, std::unique_ptr<Actor<Callback>>);
};

class GeoJSONSource : public Source {
public:

    static constexpr auto Name() { return "com/mapbox/mapboxsdk/style/sources/GeoJsonSource"; };

    static jni::Class<GeoJSONSource> javaClass;

    static void registerNative(jni::JNIEnv&);

    GeoJSONSource(jni::JNIEnv&, jni::String, jni::Object<>);

    GeoJSONSource(jni::JNIEnv&, mbgl::style::Source&, AndroidRendererFrontend&);

    ~GeoJSONSource();

    void setGeoJSONString(jni::JNIEnv&, jni::String);

    void setFeatureCollection(jni::JNIEnv&, jni::Object<geojson::FeatureCollection>);

    void setFeature(jni::JNIEnv&, jni::Object<geojson::Feature>);

    void setGeometry(jni::JNIEnv&, jni::Object<geojson::Geometry>);

    void setURL(jni::JNIEnv&, jni::String);

    jni::String getURL(jni::JNIEnv&);

    jni::Array<jni::Object<geojson::Feature>> querySourceFeatures(jni::JNIEnv&,
                                                                  jni::Array<jni::Object<>> jfilter);

private:
    jni::Object<Source> createJavaPeer(jni::JNIEnv&);
    std::unique_ptr<Update> awaitingUpdate;
    std::unique_ptr<Update> update;
    std::unique_ptr<Actor<FeatureConverter>> converter;

    template <class JNIType>
    void setAsync(jni::JNIEnv& env, jni::Object<JNIType> jObject);

}; // class GeoJSONSource

} // namespace android
} // namespace mbgl
