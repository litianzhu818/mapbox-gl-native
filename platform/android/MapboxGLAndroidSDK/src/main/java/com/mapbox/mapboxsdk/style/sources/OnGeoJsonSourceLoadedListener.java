package com.mapbox.mapboxsdk.style.sources;

import android.support.annotation.Keep;
import android.support.annotation.UiThread;

public interface OnGeoJsonSourceLoadedListener {
  @UiThread
  @Keep
  void onGeoJsonSourceLoaded();
}
