package com.googlecode.aumiks;

import android.app.NativeActivity;

public class SharedLibLoaderNativeActivity extends NativeActivity {

static {

System.loadLibrary("gnustl_shared");
System.loadLibrary("ting");
System.loadLibrary("aumiks");
System.loadLibrary("aumiks_test");
}

}
