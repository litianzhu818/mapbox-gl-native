#include "geojson_source.hpp"

#include <mbgl/renderer/query.hpp>

// Java -> C++ conversion
#include "../android_conversion.hpp"
#include "../conversion/filter.hpp"
#include <mbgl/style/conversion.hpp>
#include <mbgl/style/conversion/geojson.hpp>
#include <mbgl/style/conversion/geojson_options.hpp>

// C++ -> Java conversion
#include "../../conversion/conversion.hpp"
#include "../../conversion/collection.hpp"
#include "../../geojson/conversion/feature.hpp"
#include "../conversion/url_or_tileset.hpp"

#include <string>
#include <mbgl/util/shared_thread_pool.hpp>

namespace mbgl {
namespace android {

    // This conversion is expected not to fail because it's used only in contexts where
    // the value was originally a GeoJsonOptions object on the Java side. If it fails
    // to convert, it's a bug in our serialization or Java-side static typing.
    static style::GeoJSONOptions convertGeoJSONOptions(jni::JNIEnv& env, jni::Object<> options) {
        using namespace mbgl::style::conversion;
        if (!options) {
            return style::GeoJSONOptions();
        }
        Error error;
        optional<style::GeoJSONOptions> result = convert<style::GeoJSONOptions>(mbgl::android::Value(env, options), error);
        if (!result) {
            throw std::logic_error(error.message);
        }
        return *result;
    }

    GeoJSONSource::GeoJSONSource(jni::JNIEnv& env, jni::String sourceId, jni::Object<> options)
        : Source(env, std::make_unique<mbgl::style::GeoJSONSource>(
                jni::Make<std::string>(env, sourceId),
                convertGeoJSONOptions(env, options))
            ), converter(std::make_unique<Actor<FeatureConverter>>(*sharedThreadPool())) {
    }

    GeoJSONSource::GeoJSONSource(jni::JNIEnv& env,
                                 mbgl::style::Source& coreSource,
                                 AndroidRendererFrontend& frontend)
            : Source(env, coreSource, createJavaPeer(env), frontend)
            , converter(std::make_unique<Actor<FeatureConverter>>(*sharedThreadPool())) {
    }

    GeoJSONSource::~GeoJSONSource() = default;

    void GeoJSONSource::setGeoJSONString(jni::JNIEnv& env, jni::String json) {
        using namespace mbgl::style::conversion;

        // Convert the jni object
        Error error;
        optional<GeoJSON> converted = convert<GeoJSON>(mbgl::android::Value(env, json), error);
        if(!converted) {
            mbgl::Log::Error(mbgl::Event::JNI, "Error setting geo json: " + error.message);
            return;
        }

        // Update the core source
        source.as<mbgl::style::GeoJSONSource>()->GeoJSONSource::setGeoJSON(*converted);
    }

    void GeoJSONSource::setGeoJSONStringAsync(jni::JNIEnv& env, jni::String json,
                                              jni::Object<OnGeoJsonSourceLoadedListener> jListener) {
        // If another update is running, log an error
        if (callback) {
            mbgl::Log::Error(mbgl::Event::JNI, "Error setting GeoJSON: another asynchronous update of this source is being processed.");
            return;
        }

        callback = std::make_unique<Actor<FeatureConverter::Callback>>(
                *Scheduler::GetCurrent(),
                [this](GeoJSON geoJSON) {
                    android::UniqueEnv _env = android::AttachEnv();

                    // Update the core source
                    source.as<mbgl::style::GeoJSONSource>()->GeoJSONSource::setGeoJSON(geoJSON);

                    stringRef.release();
                    callback.release();
                    callback = nullptr;

                    // Obtain a local reference to the listener and release the global reference
                    jni::Object<OnGeoJsonSourceLoadedListener> listener = *sourceLoadedListenerRef;
                    sourceLoadedListenerRef.release();

                    // Notify the update listener
                    OnGeoJsonSourceLoadedListener::notifySourceLoaded(*_env, listener);
                });

        stringRef = json.NewGlobalRef(env);
        sourceLoadedListenerRef = jListener.NewGlobalRef(env);
        converter->self().invoke(&FeatureConverter::convertJson, *stringRef, callback->self());
    }

    void GeoJSONSource::setFeatureCollection(jni::JNIEnv& env, jni::Object<geojson::FeatureCollection> jFeatures) {
        using namespace mbgl::android::geojson;

        // Convert the jni object
        auto features = FeatureCollection::convert(env, jFeatures);

        // Update the core source
        source.as<mbgl::style::GeoJSONSource>()->GeoJSONSource::setGeoJSON(GeoJSON(features));
    }

    void GeoJSONSource::setFeatureCollectionAsync(jni::JNIEnv& env, jni::Object<geojson::FeatureCollection> jFeatures,
                                                  jni::Object<OnGeoJsonSourceLoadedListener> jListener) {
        // If another update is running, log an error
        if (callback) {
            mbgl::Log::Error(mbgl::Event::JNI, "Error setting GeoJSON: another asynchronous update of this source is being processed.");
            return;
        }

        callback = std::make_unique<Actor<FeatureConverter::Callback>>(
                *Scheduler::GetCurrent(),
                [this](GeoJSON geoJSON) {
                    android::UniqueEnv _env = android::AttachEnv();

                    // Update the core source
                    source.as<mbgl::style::GeoJSONSource>()->GeoJSONSource::setGeoJSON(geoJSON);

                    collectionRef.release();
                    callback.release();
                    callback = nullptr;

                    // Obtain a local reference to the listener and release the global reference
                    jni::Object<OnGeoJsonSourceLoadedListener> listener = *sourceLoadedListenerRef;
                    sourceLoadedListenerRef.release();

                    // Notify the update listener
                    OnGeoJsonSourceLoadedListener::notifySourceLoaded(*_env, listener);
                });

        collectionRef = jFeatures.NewGlobalRef(env);
        sourceLoadedListenerRef = jListener.NewGlobalRef(env);
        converter->self().invoke(&FeatureConverter::convertFeatureCollection, *collectionRef, callback->self());
    }

    void GeoJSONSource::setFeature(jni::JNIEnv& env, jni::Object<geojson::Feature> jFeature) {
        using namespace mbgl::android::geojson;

        // Convert the jni object
        auto feature = Feature::convert(env, jFeature);

        // Update the core source
        source.as<mbgl::style::GeoJSONSource>()->GeoJSONSource::setGeoJSON(GeoJSON(feature));
    }

    void GeoJSONSource::setFeatureAsync(JNIEnv& env, jni::Object<geojson::Feature> jFeature,
                                   jni::Object<OnGeoJsonSourceLoadedListener> jListener) {
        // If another update is running, log an error
        if (callback) {
            mbgl::Log::Error(mbgl::Event::JNI, "Error setting GeoJSON: another asynchronous update of this source is being processed.");
            return;
        }

        callback = std::make_unique<Actor<FeatureConverter::Callback>>(
                *Scheduler::GetCurrent(),
                [this](GeoJSON geoJSON) {
                    android::UniqueEnv _env = android::AttachEnv();

                    // Update the core source
                    source.as<mbgl::style::GeoJSONSource>()->GeoJSONSource::setGeoJSON(geoJSON);

                    featureRef.release();
                    callback.release();
                    callback = nullptr;

                    // Obtain a local reference to the listener and release the global reference
                    jni::Object<OnGeoJsonSourceLoadedListener> listener = *sourceLoadedListenerRef;
                    sourceLoadedListenerRef.release();

                    // Notify the update listener
                    OnGeoJsonSourceLoadedListener::notifySourceLoaded(*_env, listener);
                });

        featureRef = jFeature.NewGlobalRef(env);
        sourceLoadedListenerRef = jListener.NewGlobalRef(env);
        converter->self().invoke(&FeatureConverter::convertFeature, *featureRef, callback->self());
    }

    void GeoJSONSource::setGeometry(jni::JNIEnv& env, jni::Object<geojson::Geometry> jGeometry) {
        using namespace mbgl::android::geojson;

        // Convert the jni object
        auto geometry = Geometry::convert(env, jGeometry);

        // Update the core source
        source.as<mbgl::style::GeoJSONSource>()->GeoJSONSource::setGeoJSON(GeoJSON(geometry));
    }

    void GeoJSONSource::setGeometryAsync(jni::JNIEnv& env, jni::Object<geojson::Geometry> jGeometry,
                                         jni::Object<OnGeoJsonSourceLoadedListener> jListener) {
        // If another update is running, log an error
        if (callback) {
            mbgl::Log::Error(mbgl::Event::JNI, "Error setting GeoJSON: another asynchronous update of this source is being processed.");
            return;
        }

        callback = std::make_unique<Actor<FeatureConverter::Callback>>(
                *Scheduler::GetCurrent(),
                [this](GeoJSON geoJSON) {
                    android::UniqueEnv _env = android::AttachEnv();

                    // Update the core source
                    source.as<mbgl::style::GeoJSONSource>()->GeoJSONSource::setGeoJSON(geoJSON);

                    geometryRef.release();
                    callback.release();
                    callback = nullptr;

                    // Obtain a local reference to the listener and release the global reference
                    jni::Object<OnGeoJsonSourceLoadedListener> listener = *sourceLoadedListenerRef;
                    sourceLoadedListenerRef.release();

                    // Notify the update listener
                    OnGeoJsonSourceLoadedListener::notifySourceLoaded(*_env, listener);
                });

        geometryRef = jGeometry.NewGlobalRef(env);
        sourceLoadedListenerRef = jListener.NewGlobalRef(env);
        converter->self().invoke(&FeatureConverter::convertGeometry, *geometryRef, callback->self());
    }

    void GeoJSONSource::setURL(jni::JNIEnv& env, jni::String url) {
        // Update the core source
        source.as<mbgl::style::GeoJSONSource>()->GeoJSONSource::setURL(jni::Make<std::string>(env, url));
    }

    jni::String GeoJSONSource::getURL(jni::JNIEnv& env) {
        optional<std::string> url = source.as<mbgl::style::GeoJSONSource>()->GeoJSONSource::getURL();
        return url ? jni::Make<jni::String>(env, *url) : jni::String();
    }

    jni::Array<jni::Object<geojson::Feature>> GeoJSONSource::querySourceFeatures(jni::JNIEnv& env,
                                                                        jni::Array<jni::Object<>> jfilter) {
        using namespace mbgl::android::conversion;
        using namespace mbgl::android::geojson;

        std::vector<mbgl::Feature> features;
        if (rendererFrontend) {
            features = rendererFrontend->querySourceFeatures(source.getID(), { {},  toFilter(env, jfilter) });
        }
        return *convert<jni::Array<jni::Object<Feature>>, std::vector<mbgl::Feature>>(env, features);
    }

    jni::Class<GeoJSONSource> GeoJSONSource::javaClass;

    jni::Object<Source> GeoJSONSource::createJavaPeer(jni::JNIEnv& env) {
        static auto constructor = GeoJSONSource::javaClass.template GetConstructor<jni::jlong>(env);
        return jni::Object<Source>(GeoJSONSource::javaClass.New(env, constructor, reinterpret_cast<jni::jlong>(this)).Get());
    }

    void GeoJSONSource::registerNative(jni::JNIEnv& env) {
        // Lookup the class
        GeoJSONSource::javaClass = *jni::Class<GeoJSONSource>::Find(env).NewGlobalRef(env).release();

        #define METHOD(MethodPtr, name) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)

        // Register the peer
        jni::RegisterNativePeer<GeoJSONSource>(
            env, GeoJSONSource::javaClass, "nativePtr",
            std::make_unique<GeoJSONSource, JNIEnv&, jni::String, jni::Object<>>,
            "initialize",
            "finalize",
            METHOD(&GeoJSONSource::setGeoJSONString, "nativeSetGeoJsonString"),
            METHOD(&GeoJSONSource::setGeoJSONStringAsync, "nativeSetGeoJsonStringAsync"),
            METHOD(&GeoJSONSource::setFeatureCollection, "nativeSetFeatureCollection"),
            METHOD(&GeoJSONSource::setFeatureCollectionAsync, "nativeSetFeatureCollectionAsync"),
            METHOD(&GeoJSONSource::setFeature, "nativeSetFeature"),
            METHOD(&GeoJSONSource::setFeatureAsync, "nativeSetFeatureAsync"),
            METHOD(&GeoJSONSource::setGeometry, "nativeSetGeometry"),
            METHOD(&GeoJSONSource::setGeometryAsync, "nativeSetGeometryAsync"),
            METHOD(&GeoJSONSource::setURL, "nativeSetUrl"),
            METHOD(&GeoJSONSource::getURL, "nativeGetUrl"),
            METHOD(&GeoJSONSource::querySourceFeatures, "querySourceFeatures")
        );
    }

    void FeatureConverter::convertFeatureCollection(jni::Object<geojson::FeatureCollection> jFeatures,
                                                    ActorRef<FeatureConverter::Callback> callback) {
        using namespace mbgl::android::geojson;

        android::UniqueEnv _env = android::AttachEnv();
        // Convert the jni object
        auto features = FeatureCollection::convert(*_env, jFeatures);
        callback.invoke(&FeatureConverter::Callback::operator(), GeoJSON(features));
    }

    void FeatureConverter::convertFeature(jni::Object<geojson::Feature> jFeature,
                                          ActorRef<FeatureConverter::Callback> callback) {
        using namespace mbgl::android::geojson;

        android::UniqueEnv _env = android::AttachEnv();
        // Convert the jni object
        auto feature = Feature::convert(*_env, jFeature);
        callback.invoke(&FeatureConverter::Callback::operator(), GeoJSON(feature));
    }

    void FeatureConverter::convertGeometry(jni::Object<geojson::Geometry> jGeometry,
                                           ActorRef<FeatureConverter::Callback> callback) {
        using namespace mbgl::android::geojson;

        android::UniqueEnv _env = android::AttachEnv();
        // Convert the jni object
        auto geometry = Geometry::convert(*_env, jGeometry);
        callback.invoke(&FeatureConverter::Callback::operator(), GeoJSON(geometry));
    }

    void FeatureConverter::convertJson(jni::String json,
                                       ActorRef<FeatureConverter::Callback> callback) {
        using namespace mbgl::style::conversion;

        android::UniqueEnv _env = android::AttachEnv();

        // Convert the jni object
        Error error;
        optional<GeoJSON> converted = convert<GeoJSON>(mbgl::android::Value(*_env, json), error);
        if(!converted) {
            mbgl::Log::Error(mbgl::Event::JNI, "Error setting geo json: " + error.message);
            return;
        }

        callback.invoke(&FeatureConverter::Callback::operator(), *converted);
    }

} // namespace android
} // namespace mbgl
