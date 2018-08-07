#pragma once

#include "source.hpp"
#include <mbgl/style/sources/geojson_source.hpp>
#include "../../geojson/geometry.hpp"
#include "../../geojson/feature.hpp"
#include "../../geojson/feature_collection.hpp"
#include "on_geojson_source_loaded_listener.hpp"
#include <jni/jni.hpp>

namespace mbgl {
namespace android {

class FeatureConverter {
public:
    using Callback = std::function<void (GeoJSON)>;
    void convertFeatureCollection(jni::Object<geojson::FeatureCollection>, ActorRef<Callback>);
    void convertFeature(jni::Object<geojson::Feature>, ActorRef<Callback>);
    void convertGeometry(jni::Object<geojson::Geometry>, ActorRef<Callback>);
    void convertJson(jni::String, ActorRef<Callback>);
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

    void setGeoJSONStringAsync(jni::JNIEnv&, jni::String, jni::Object<OnGeoJsonSourceLoadedListener>);

    void setFeatureCollection(jni::JNIEnv&, jni::Object<geojson::FeatureCollection>);

    void setFeatureCollectionAsync(jni::JNIEnv&, jni::Object<geojson::FeatureCollection>, jni::Object<OnGeoJsonSourceLoadedListener>);

    void setFeature(jni::JNIEnv&, jni::Object<geojson::Feature>);

    void setFeatureAsync(jni::JNIEnv&, jni::Object<geojson::Feature>, jni::Object<OnGeoJsonSourceLoadedListener>);

    void setGeometry(jni::JNIEnv&, jni::Object<geojson::Geometry>);

    void setGeometryAsync(jni::JNIEnv&, jni::Object<geojson::Geometry>, jni::Object<OnGeoJsonSourceLoadedListener>);

    void setURL(jni::JNIEnv&, jni::String);

    jni::Array<jni::Object<geojson::Feature>> querySourceFeatures(jni::JNIEnv&,
                                                                  jni::Array<jni::Object<>> jfilter);

    jni::String getURL(jni::JNIEnv&);

private:
    jni::Object<Source> createJavaPeer(jni::JNIEnv&);
    jni::UniqueObject<geojson::FeatureCollection> collectionRef;
    jni::UniqueObject<geojson::Feature> featureRef;
    jni::UniqueObject<geojson::Geometry> geometryRef;
    jni::UniqueObject<jni::StringTag> stringRef;
    std::unique_ptr<Actor<FeatureConverter::Callback>> callback;
    jni::UniqueObject<OnGeoJsonSourceLoadedListener> sourceLoadedListenerRef;
    std::unique_ptr<Actor<FeatureConverter>> converter;

}; // class GeoJSONSource

} // namespace android
} // namespace mbgl
