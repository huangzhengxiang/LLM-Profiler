plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace = "com.iot.audio"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.iot.audio"
        minSdk = 31
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
            externalNativeBuild {
            cmake {
                arguments "-DANDROID_STL=c++_shared"
                abiFilters 'arm64-v8a'
            }
        }
    }

    externalNativeBuild {
        cmake {
            path file('../../CMakeLists.txt')
        }
    }
    sourceSets {
        main {
            jniLibs.srcDirs = ['src/main/jni/libs']
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
}

dependencies {

    implementation(libs.appcompat)
    implementation(libs.material)
    implementation(libs.activity)
    implementation(libs.constraintlayout)
    testImplementation(libs.junit)
    androidTestImplementation(libs.ext.junit)
    androidTestImplementation(libs.espresso.core)
}