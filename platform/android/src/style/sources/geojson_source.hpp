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
    void convertJson(std::string, ActorRef<Callback>);

    template <class JNIType>
    void convertObject(jni::Object<JNIType>, ActorRef<Callback>);
};

struct Update {
    std::unique_ptr<Actor<Callback>> callback;

    using Updater = std::function<void (ActorRef<FeatureConverter>)>;
    Updater updateFn;

    virtual ~Update();

    virtual void makeUpdateFn() {};
};

struct CollectionUpdate : Update {
    std::shared_ptr<jni::jobject> object;

    CollectionUpdate(std::shared_ptr<jni::jobject>, std::unique_ptr<Actor<Callback>>);

    template <class JNIType>
    void makeUpdateFn();
};

struct StringUpdate : Update {
    std::string json;

    StringUpdate(std::string, std::unique_ptr<Actor<Callback>>);

    void makeUpdateFn();
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
    void setCollectionAsync(jni::JNIEnv&, jni::Object<JNIType>);

    template <class UpdateType, typename ObjectType>
    std::unique_ptr<UpdateType> makeUpdate(ObjectType object);

    void setAsync(std::unique_ptr<Update>);

}; // class GeoJSONSource

} // namespace android
} // namespace mbgl
