package com.cloudwebrtc.webrtc;

import android.Manifest;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.ResultReceiver;
import android.util.Log;
import android.content.Intent;
import android.app.Activity;

import android.media.projection.MediaProjection;
import android.media.projection.MediaProjectionManager;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.webrtc.*;

import io.flutter.plugin.common.MethodChannel.Result;

/**
 * The implementation of {@code getUserMedia} extracted into a separate file in
 * order to reduce complexity and to (somewhat) separate concerns.
 */
class GetUserMediaImpl{
    private static final int DEFAULT_WIDTH  = 1280;
    private static final int DEFAULT_HEIGHT = 720;
    private static final int DEFAULT_FPS    = 30;

    private static final String PERMISSION_AUDIO = Manifest.permission.RECORD_AUDIO;
    private static final String PERMISSION_VIDEO = Manifest.permission.CAMERA;
    private static final String PERMISSION_SCREEN = "android.permission.MediaProjection";
    private static int CAPTURE_PERMISSION_REQUEST_CODE = 1;
    private static final String GRANT_RESULTS = "GRANT_RESULT";
    private static final String PERMISSIONS = "PERMISSION";
    private static final String PROJECTION_DATA = "PROJECTION_DATA";
    private static final String RESULT_RECEIVER = "RESULT_RECEIVER";
    private static final String REQUEST_CODE = "REQUEST_CODE";

    static final String TAG = FlutterWebRTCPlugin.TAG;

    private final Map<String, VideoCapturer> mVideoCapturers
        = new HashMap<String, VideoCapturer>();

    private final Context applicationContext;
    private final FlutterWebRTCPlugin plugin;

    static final int minAPILevel = Build.VERSION_CODES.LOLLIPOP;
    private MediaProjectionManager mProjectionManager = null;
    private static MediaProjection sMediaProjection = null;

    public void screenRequestPremissions(ResultReceiver resultReceiver){
        Activity activity = plugin.getActivity();

        Bundle args = new Bundle();
        args.putParcelable(RESULT_RECEIVER, resultReceiver);
        args.putInt(REQUEST_CODE, CAPTURE_PERMISSION_REQUEST_CODE);

        ScreenRequestPermissionsFragment fragment = new ScreenRequestPermissionsFragment();
        fragment.setArguments(args);

        FragmentTransaction transaction
                = activity.getFragmentManager().beginTransaction().add(
                fragment,
                fragment.getClass().getName());

        try {
            transaction.commit();
        } catch (IllegalStateException ise) {

        }
    }

    public static class ScreenRequestPermissionsFragment extends Fragment {

        private  ResultReceiver resultReceiver = null;
        private  int requestCode = 0;
        private int resultCode = 0;

        private void checkSelfPermissions(boolean requestPermissions) {
            if(resultCode != Activity.RESULT_OK) {
                Activity activity = this.getActivity();
                Bundle args = getArguments();
                resultReceiver = (ResultReceiver) args.getParcelable(RESULT_RECEIVER);
                requestCode = args.getInt(REQUEST_CODE);
                requestStart(activity, requestCode);
            }
        }

        public void requestStart(Activity activity, int requestCode) {
            if (android.os.Build.VERSION.SDK_INT < minAPILevel) {
                Log.w(TAG, "Can't run requestStart() due to a low API level. API level 21 or higher is required.");
                return;
            } else {
                MediaProjectionManager mediaProjectionManager =
                        (MediaProjectionManager) activity.getSystemService(
                                Context.MEDIA_PROJECTION_SERVICE);

                // call for the projection manager
                this.startActivityForResult(
                        mediaProjectionManager.createScreenCaptureIntent(), requestCode);
            }
        }


        @Override
        public void onActivityResult(int requestCode, int resultCode, Intent data) {
            super.onActivityResult(requestCode, resultCode, data);
            resultCode = resultCode;
            String[] permissions;
            if (resultCode != Activity.RESULT_OK) {
                finish();
                Bundle resultData = new Bundle();
                resultData.putString(PERMISSIONS, PERMISSION_SCREEN);
                resultData.putInt(GRANT_RESULTS, resultCode);
                resultReceiver.send(requestCode, resultData);
                return;
            }
            Bundle resultData = new Bundle();
            resultData.putString(PERMISSIONS, PERMISSION_SCREEN);
            resultData.putInt(GRANT_RESULTS, resultCode);
            resultData.putParcelable(PROJECTION_DATA, data);
            resultReceiver.send(requestCode, resultData);
            finish();
        }

        private void finish() {
            Activity activity = getActivity();
            if (activity != null) {
                activity.getFragmentManager().beginTransaction()
                        .remove(this)
                        .commitAllowingStateLoss();
            }
        }

        @Override
        public void onResume() {
            super.onResume();

            checkSelfPermissions(/* requestPermissions */ true);
        }
    }

    GetUserMediaImpl(
            FlutterWebRTCPlugin plugin,
            Context applicationContext) {
         this.plugin = plugin;
         this.applicationContext = applicationContext;
    }

    /**
     * Includes default constraints set for the audio media type.
     * @param audioConstraints <tt>MediaConstraints</tt> instance to be filled
     * with the default constraints for audio media type.
     */
    private void addDefaultAudioConstraints(MediaConstraints audioConstraints) {
        audioConstraints.optional.add(
            new MediaConstraints.KeyValuePair("googNoiseSuppression", "true"));
        audioConstraints.optional.add(
            new MediaConstraints.KeyValuePair("googEchoCancellation", "true"));
        audioConstraints.optional.add(
            new MediaConstraints.KeyValuePair("echoCancellation", "true"));
        audioConstraints.optional.add(
            new MediaConstraints.KeyValuePair("googEchoCancellation2", "true"));
        audioConstraints.optional.add(
            new MediaConstraints.KeyValuePair(
                    "googDAEchoCancellation", "true"));
    }

    /**
     * Create video capturer via given facing mode
     * @param enumerator a <tt>CameraEnumerator</tt> provided by webrtc
     *        it can be Camera1Enumerator or Camera2Enumerator
     * @param isFacing 'user' mapped with 'front' is true (default)
     *                 'environment' mapped with 'back' is false
     * @param sourceId (String) use this sourceId and ignore facing mode if specified.
     * @return VideoCapturer can invoke with <tt>startCapture</tt>/<tt>stopCapture</tt>
     *         <tt>null</tt> if not matched camera with specified facing mode.
     */
    private VideoCapturer createVideoCapturer(
            CameraEnumerator enumerator,
            boolean isFacing,
            String sourceId) {
        VideoCapturer videoCapturer = null;

        // if sourceId given, use specified sourceId first
        final String[] deviceNames = enumerator.getDeviceNames();
        if (sourceId != null) {
            for (String name : deviceNames) {
                if (name.equals(sourceId)) {
                    videoCapturer = enumerator.createCapturer(name, new CameraEventsHandler());
                    if (videoCapturer != null) {
                        Log.d(TAG, "create user specified camera " + name + " succeeded");
                        return videoCapturer;
                    } else {
                        Log.d(TAG, "create user specified camera " + name + " failed");
                        break; // fallback to facing mode
                    }
                }
            }
        }

        // otherwise, use facing mode
        String facingStr = isFacing ? "front" : "back";
        for (String name : deviceNames) {
            if (enumerator.isFrontFacing(name) == isFacing) {
                videoCapturer = enumerator.createCapturer(name, new CameraEventsHandler());
                if (videoCapturer != null) {
                    Log.d(TAG, "Create " + facingStr + " camera " + name + " succeeded");
                    return videoCapturer;
                } else {
                    Log.e(TAG, "Create " + facingStr + " camera " + name + " failed");
                }
            }
        }
        // should we fallback to available camera automatically?
        return videoCapturer;
    }

    /**
     * Retrieves "facingMode" constraint value.
     *
     * @param mediaConstraints a <tt>ConstraintsMap</tt> which represents "GUM"
     * constraints argument.
     * @return String value of "facingMode" constraints in "GUM" or
     * <tt>null</tt> if not specified.
     */
    private String getFacingMode(ConstraintsMap mediaConstraints) {
        return
            mediaConstraints == null
                ? null
                : mediaConstraints.getString("facingMode");
    }

    /**
     * Retrieves "sourceId" constraint value.
     *
     * @param mediaConstraints a <tt>ConstraintsMap</tt> which represents "GUM"
     * constraints argument
     * @return String value of "sourceId" optional "GUM" constraint or
     * <tt>null</tt> if not specified.
     */
    private String getSourceIdConstraint(ConstraintsMap mediaConstraints) {
        if (mediaConstraints != null
                && mediaConstraints.hasKey("optional")
                && mediaConstraints.getType("optional") == ObjectType.Array) {
            ConstraintsArray optional = mediaConstraints.getArray("optional");

            for (int i = 0, size = optional.size(); i < size; i++) {
                if (optional.getType(i) == ObjectType.Map) {
                    ConstraintsMap option = optional.getMap(i);

                    if (option.hasKey("sourceId")
                            && option.getType("sourceId")
                                == ObjectType.String) {
                        return option.getString("sourceId");
                    }
                }
            }
        }

        return null;
    }

    private AudioTrack getUserAudio(ConstraintsMap constraints) {
        MediaConstraints audioConstraints;
        if (constraints.getType("audio") == ObjectType.Boolean) {
            audioConstraints = new MediaConstraints();
            addDefaultAudioConstraints(audioConstraints);
        } else {
            audioConstraints
                = plugin.parseMediaConstraints(
                    constraints.getMap("audio"));
        }

        Log.i(TAG, "getUserMedia(audio): " + audioConstraints);

        String trackId = plugin.getNextTrackUUID();
        PeerConnectionFactory pcFactory = plugin.mFactory;
        AudioSource audioSource = pcFactory.createAudioSource(audioConstraints);

        return pcFactory.createAudioTrack(trackId, audioSource);
    }

    void getImageMedia(
            final ConstraintsMap constraints,
            final Result promise,
            final MediaStream mediaStream) {

        ConstraintsArray tracks_ =  new ConstraintsArray();
        ConstraintsArray successResult = new ConstraintsArray();
        MediaStreamTrack[] tracks = new MediaStreamTrack[1];


        ImageCapturer videoCapturer = new ImageCapturer();
        PeerConnectionFactory pcFactory = plugin.mFactory;
        VideoSource videoSource = pcFactory.createVideoSource(videoCapturer);

        String trackId = plugin.getNextTrackUUID();
        mVideoCapturers.put(trackId, videoCapturer);

        tracks[0] = pcFactory.createVideoTrack(trackId, videoSource);

        for (MediaStreamTrack track : tracks) {
            if (track == null) {
                continue;
            }

            String id = track.id();

            if (track instanceof AudioTrack) {
                mediaStream.addTrack((AudioTrack) track);
            } else {
                mediaStream.addTrack((VideoTrack) track);
            }
            plugin.localTracks.put(id, track);

            ConstraintsMap track_ = new ConstraintsMap();
            String kind = track.kind();

            track_.putBoolean("enabled", track.enabled());
            track_.putString("id", id);
            track_.putString("kind", kind);
            track_.putString("label", kind);
            track_.putString("readyState", track.state().toString());
            track_.putBoolean("remote", false);
            tracks_.pushMap(track_);
        }

        String streamId = mediaStream.label();

        Log.d(TAG, "MediaStream id: " + streamId);
        plugin.localStreams.put(streamId, mediaStream);

        successResult.pushString(streamId);
        successResult.pushArray(tracks_);
        promise.success(successResult.toArrayList());
    }

    /**
     * Implements {@code getUserMedia} without knowledge whether the necessary
     * permissions have already been granted. If the necessary permissions have
     * not been granted yet, they will be requested.
     */
    void getUserMedia(
            final ConstraintsMap constraints,
            final Result promise,
            final MediaStream mediaStream) {
        // TODO: change getUserMedia constraints format to support new syntax
        //   constraint format seems changed, and there is no mandatory any more.
        //   and has a new syntax/attrs to specify resolution
        //   should change `parseConstraints()` according
        //   see: https://www.w3.org/TR/mediacapture-streams/#idl-def-MediaTrackConstraints

        ConstraintsMap  videoConstraintsMap = null;
        ConstraintsMap  videoConstraintsMandatory = null;

        if (constraints.getType("video") == ObjectType.Map) {
            videoConstraintsMap = constraints.getMap("video");
            if (videoConstraintsMap.hasKey("mandatory")
                    && videoConstraintsMap.getType("mandatory")
                    == ObjectType.Map) {
                videoConstraintsMandatory
                        = videoConstraintsMap.getMap("mandatory");
            }
        }

        boolean requestScreenCapturer =  videoConstraintsMandatory.hasKey("chromeMediaSource") &&
                videoConstraintsMandatory.getString("chromeMediaSource").equals("desktop");

        final ArrayList<String> requestPermissions = new ArrayList<>();

        if(requestScreenCapturer)
        {
            final  ConstraintsMap videoConstraintsMandatory2 =  videoConstraintsMandatory;
            screenRequestPremissions(new ResultReceiver(new Handler(Looper.getMainLooper())) {
                @Override
                protected void onReceiveResult(
                        int requestCode,
                        Bundle resultData) {

                    /*Create ScreenCapture*/
                    int resultCode = resultData.getInt(GRANT_RESULTS);
                    Intent mediaProjectionData = resultData.getParcelable(PROJECTION_DATA);

                    if (resultCode != Activity.RESULT_OK) {
                        promise.error( null,
                                "User didn't give permission to capture the screen.", null);
                        return;
                    }

                    ConstraintsArray tracks_ = new ConstraintsArray();
                    ConstraintsArray successResult = new ConstraintsArray();
                    MediaStreamTrack[] tracks = new MediaStreamTrack[1];
                    VideoCapturer videoCapturer = null;
                    videoCapturer = new ScreenCapturerAndroid(
                            mediaProjectionData, new MediaProjection.Callback() {
                        @Override
                        public void onStop() {
                            Log.e(TAG, "User revoked permission to capture the screen.");
                            promise.error(  null,
                                    "User revoked permission to capture the screen.", null);
                        }
                    });

                    if (videoCapturer != null) {


                        PeerConnectionFactory pcFactory = plugin.mFactory;
                        VideoSource videoSource = pcFactory.createVideoSource(videoCapturer);

                        // Fall back to defaults if keys are missing.
                        int width
                                = videoConstraintsMandatory2.hasKey("minWidth")
                                ? videoConstraintsMandatory2.getInt("minWidth")
                                : DEFAULT_WIDTH;
                        int height
                                = videoConstraintsMandatory2.hasKey("minHeight")
                                ? videoConstraintsMandatory2.getInt("minHeight")
                                : DEFAULT_HEIGHT;
                        int fps
                                = videoConstraintsMandatory2.hasKey("minFrameRate")
                                ? videoConstraintsMandatory2.getInt("minFrameRate")
                                : DEFAULT_FPS;

                        videoCapturer.startCapture(width, height, fps);

                        String trackId = plugin.getNextTrackUUID();
                        mVideoCapturers.put(trackId, videoCapturer);

                        Log.d(TAG, "changeCaptureFormat: " + width + "x" + height + "@" + fps);
                        videoSource.adaptOutputFormat(width, height, fps);

                        tracks[0] = pcFactory.createVideoTrack(trackId, videoSource);

                        for (MediaStreamTrack track : tracks) {
                            if (track == null) {
                                continue;
                            }

                            String id = track.id();

                            if (track instanceof AudioTrack) {
                                mediaStream.addTrack((AudioTrack) track);
                            } else {
                                mediaStream.addTrack((VideoTrack) track);
                            }
                            plugin.localTracks.put(id, track);

                            ConstraintsMap track_ = new ConstraintsMap();
                            String kind = track.kind();

                            track_.putBoolean("enabled", track.enabled());
                            track_.putString("id", id);
                            track_.putString("kind", kind);
                            track_.putString("label", kind);
                            track_.putString("readyState", track.state().toString());
                            track_.putBoolean("remote", false);
                            tracks_.pushMap(track_);
                        }

                        String streamId = mediaStream.label();

                        Log.d(TAG, "MediaStream id: " + streamId);
                        plugin.localStreams.put(streamId, mediaStream);

                        successResult.pushString(streamId);
                        successResult.pushArray(tracks_);
                        promise.success(successResult);
                    }

                }
            });
            return;
        }

        if (constraints.hasKey("audio")) {
            switch (constraints.getType("audio")) {
            case Boolean:
                if (constraints.getBoolean("audio")) {
                    requestPermissions.add(PERMISSION_AUDIO);
                }
                break;
            case Map:
                requestPermissions.add(PERMISSION_AUDIO);
                break;
            default:
                break;
            }
        }

        if (constraints.hasKey("video") && !requestScreenCapturer) {
            switch (constraints.getType("video")) {
            case Boolean:
                if (constraints.getBoolean("video")) {
                    requestPermissions.add(PERMISSION_VIDEO);
                }
                break;
            case Map:
                requestPermissions.add(PERMISSION_VIDEO);
                break;
            default:
                break;
            }
        }

        // According to step 2 of the getUserMedia() algorithm,
        // requestedMediaTypes is the set of media types in constraints with
        // either a dictionary value or a value of "true".
        // According to step 3 of the getUserMedia() algorithm, if
        // requestedMediaTypes is the empty set, the method invocation fails
        // with a TypeError.
        if (requestPermissions.isEmpty()) {
            promise.error(
                "TypeError",
                "constraints requests no media types", null);
            return;
        }

        requestPermissions(
            requestPermissions,
            /* successCallback */ new Callback() {
                @Override
                public void invoke(Object... args) {
                    List<String> grantedPermissions = (List<String>) args[0];

                    getUserMedia(
                        constraints,
                        promise,
                        mediaStream,
                        grantedPermissions);
                }
            },
            /* errorCallback */ new Callback() {
                @Override
                public void invoke(Object... args) {
                    // According to step 10 Permission Failure of the
                    // getUserMedia() algorithm, if the user has denied
                    // permission, fail "with a new DOMException object whose
                    // name attribute has the value NotAllowedError."
                    promise.error("DOMException", "NotAllowedError", null);
                }
            }
        );
    }

    /**
     * Implements {@code getUserMedia} with the knowledge that the necessary
     * permissions have already been granted. If the necessary permissions have
     * not been granted yet, they will NOT be requested.
     */
    private void getUserMedia(
            ConstraintsMap constraints,
            Result promise,
            MediaStream mediaStream,
            List<String> grantedPermissions) {
        MediaStreamTrack[] tracks = new MediaStreamTrack[2];

        // If we fail to create either, destroy the other one and fail.
        if ((grantedPermissions.contains(PERMISSION_AUDIO)
                && (tracks[0] = getUserAudio(constraints)) == null)
                || (grantedPermissions.contains(PERMISSION_VIDEO)
                && (tracks[1] = getUserVideo(constraints)) == null)) {
            for (MediaStreamTrack track : tracks) {
                if (track != null) {
                    track.dispose();
                }
            }

            // XXX The following does not follow the getUserMedia() algorithm
            // specified by
            // https://www.w3.org/TR/mediacapture-streams/#dom-mediadevices-getusermedia
            // with respect to distinguishing the various causes of failure.
            promise.error(
                 /* type */ null,
                    "Failed to create new track", null);
            return;
        }

        ConstraintsArray tracks_ = new ConstraintsArray();
        ConstraintsArray successResult = new ConstraintsArray();

        for (MediaStreamTrack track : tracks) {
            if (track == null) {
                continue;
            }

            String id = track.id();

            if (track instanceof AudioTrack) {
                mediaStream.addTrack((AudioTrack) track);
            } else {
                mediaStream.addTrack((VideoTrack) track);
            }
            plugin.localTracks.put(id, track);

            ConstraintsMap track_ = new ConstraintsMap();
            String kind = track.kind();

            track_.putBoolean("enabled", track.enabled());
            track_.putString("id", id);
            track_.putString("kind", kind);
            track_.putString("label", kind);
            track_.putString("readyState", track.state().toString());
            track_.putBoolean("remote", false);
            tracks_.pushMap(track_);
        }

        String streamId = mediaStream.label();

        Log.d(TAG, "MediaStream id: " + streamId);
        plugin.localStreams.put(streamId, mediaStream);

        successResult.pushString(streamId);
        successResult.pushArray(tracks_);
        promise.success(successResult);
    }


    private VideoTrack getUserVideo(ConstraintsMap constraints) {
        ConstraintsMap videoConstraintsMap = null;
        ConstraintsMap videoConstraintsMandatory = null;
        if (constraints.getType("video") == ObjectType.Map) {
            videoConstraintsMap = constraints.getMap("video");
            if (videoConstraintsMap.hasKey("mandatory")
                    && videoConstraintsMap.getType("mandatory")
                        == ObjectType.Map) {
                videoConstraintsMandatory
                    = videoConstraintsMap.getMap("mandatory");
            }
        }

        Log.i(TAG, "getUserMedia(video): " + videoConstraintsMap);

            // NOTE: to support Camera2, the device should:
            //   1. Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP
            //   2. all camera support level should greater than LEGACY
            //   see: https://developer.android.com/reference/android/hardware/camera2/CameraCharacteristics.html#INFO_SUPPORTED_HARDWARE_LEVEL
            // TODO Enable camera2 enumerator
            Context context = plugin.getContext();
            CameraEnumerator cameraEnumerator;

            if (Camera2Enumerator.isSupported(context)) {
                Log.d(TAG, "Creating video capturer using Camera2 API.");
                cameraEnumerator = new Camera2Enumerator(context);
            } else {
                Log.d(TAG, "Creating video capturer using Camera1 API.");
                cameraEnumerator = new Camera1Enumerator(false);
            }

            String facingMode = getFacingMode(videoConstraintsMap);
            boolean isFacing
                    = facingMode == null || !facingMode.equals("environment");
        String sourceId = getSourceIdConstraint(videoConstraintsMap);

        VideoCapturer videoCapturer
                = createVideoCapturer(cameraEnumerator, isFacing, sourceId);

        if (videoCapturer == null) {
            return null;
        }

        PeerConnectionFactory pcFactory = plugin.mFactory;
        VideoSource videoSource = pcFactory.createVideoSource(videoCapturer);

        // Fall back to defaults if keys are missing.
        int width
            = videoConstraintsMandatory.hasKey("minWidth")
                ? videoConstraintsMandatory.getInt("minWidth")
                : DEFAULT_WIDTH;
        int height
            = videoConstraintsMandatory.hasKey("minHeight")
                ? videoConstraintsMandatory.getInt("minHeight")
                : DEFAULT_HEIGHT;
        int fps
            = videoConstraintsMandatory.hasKey("minFrameRate")
                ? videoConstraintsMandatory.getInt("minFrameRate")
                : DEFAULT_FPS;

        videoCapturer.startCapture(width, height, fps);

        String trackId = plugin.getNextTrackUUID();
        mVideoCapturers.put(trackId, videoCapturer);

        Log.d(TAG, "changeCaptureFormat: " + width + "x" + height + "@" + fps);
        videoSource.adaptOutputFormat(width, height, fps);

        return pcFactory.createVideoTrack(trackId, videoSource);
    }

    void removeVideoCapturer(String id) {
        VideoCapturer videoCapturer = mVideoCapturers.get(id);
        if (videoCapturer != null) {
            try {
                videoCapturer.stopCapture();
            } catch (InterruptedException e) {
                Log.e(TAG, "removeVideoCapturer() Failed to stop video capturer");
            }
            mVideoCapturers.remove(id);
        }
    }

    private void requestPermissions(
            final ArrayList<String> permissions,
            final Callback successCallback,
            final Callback errorCallback) {
        PermissionUtils.Callback callback = new PermissionUtils.Callback() {
            @Override
            public void invoke(String[] permissions_, int[] grantResults) {
                List<String> grantedPermissions = new ArrayList<>();
                List<String> deniedPermissions = new ArrayList<>();

                for (int i = 0; i < permissions_.length; ++i) {
                    String permission = permissions_[i];
                    int grantResult = grantResults[i];

                    if (grantResult == PackageManager.PERMISSION_GRANTED) {
                        grantedPermissions.add(permission);
                    } else {
                        deniedPermissions.add(permission);
                    }
                }

                // Success means that all requested permissions were granted.
                for (String p : permissions) {
                    if (!grantedPermissions.contains(p)) {
                        // According to step 6 of the getUserMedia() algorithm
                        // "if the result is denied, jump to the step Permission
                        // Failure."
                        errorCallback.invoke(deniedPermissions);
                        return;
                    }
                }
                successCallback.invoke(grantedPermissions);
            }
        };

        PermissionUtils.requestPermissions(
            plugin,
            permissions.toArray(new String[permissions.size()]),
            callback);
    }

    void switchCamera(String id) {
        VideoCapturer videoCapturer = mVideoCapturers.get(id);
        if (videoCapturer != null) {
            CameraVideoCapturer cameraVideoCapturer
                = (CameraVideoCapturer) videoCapturer;
            cameraVideoCapturer.switchCamera(null);
        }
    }

    void putImage(String id, String base64_image) {
        VideoCapturer videoCapturer = (VideoCapturer) mVideoCapturers.get(id);
        if (videoCapturer.getClass() == ImageCapturer.class) {
            ((ImageCapturer)videoCapturer).putImage(base64_image);
        }
    }
}
