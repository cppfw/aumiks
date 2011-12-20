package com.googlecode.ting;

import android.app.NativeActivity;

public class SharedLibLoaderNativeActivity extends NativeActivity {

static {

System.loadLibrary("gnustl_shared");
System.loadLibrary("ting_test");
}

}
