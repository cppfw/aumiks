package com.googlecode.aumiks;

import android.app.NativeActivity;

public class SharedLibLoaderNativeActivity extends NativeActivity {

static {

System.loadLibrary("gnustl_shared");
System.loadLibrary("aumiks_test");
}

}
