package com.cloudwebrtc.webrtc;

import java.io.UnsupportedEncodingException;
import java.lang.ref.SoftReference;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import android.util.Base64;
import android.util.Log;
import android.util.SparseArray;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import com.cloudwebrtc.webrtc.utils.JavaOnlyArray;
import com.cloudwebrtc.webrtc.utils.JavaOnlyMap;
import com.cloudwebrtc.webrtc.utils.ReadableMap;
import com.cloudwebrtc.webrtc.utils.WritableMap;
import com.cloudwebrtc.webrtc.utils.WritableArray;

import org.webrtc.AudioTrack;
import org.webrtc.DataChannel;
import org.webrtc.IceCandidate;
import org.webrtc.MediaStream;
import org.webrtc.MediaStreamTrack;
import org.webrtc.PeerConnection;
import org.webrtc.RtpReceiver;
import org.webrtc.StatsObserver;
import org.webrtc.StatsReport;
import org.webrtc.VideoTrack;

import io.flutter.plugin.common.MethodChannel.Result;

class PeerConnectionObserver implements PeerConnection.Observer {
    private final static String TAG = FlutterWebRTCPlugin.TAG;

    private final SparseArray<DataChannel> dataChannels
        = new SparseArray<DataChannel>();
    private final int id;
    private PeerConnection peerConnection;
    final Map<String, MediaStream> remoteStreams;
    final Map<String, MediaStreamTrack> remoteTracks;
    private final FlutterWebRTCPlugin plugin;

    /**
     * The <tt>StringBuilder</tt> cache utilized by {@link #statsToJSON} in
     * order to minimize the number of allocations of <tt>StringBuilder</tt>
     * instances and, more importantly, the allocations of its <tt>char</tt>
     * buffer in an attempt to improve performance.
     */
    private SoftReference<StringBuilder> statsToJSONStringBuilder
        = new SoftReference(null);

    PeerConnectionObserver(FlutterWebRTCPlugin plugin, int id) {
        this.plugin = plugin;
        this.id = id;
        this.remoteStreams = new HashMap<String, MediaStream>();
        this.remoteTracks = new HashMap<String, MediaStreamTrack>();
    }

    PeerConnection getPeerConnection() {
        return peerConnection;
    }

    void setPeerConnection(PeerConnection peerConnection) {
        this.peerConnection = peerConnection;
    }

    void close() {
         peerConnection.close();

         remoteStreams.clear();
         remoteTracks.clear();

         // Unlike on iOS, we cannot unregister the DataChannel.Observer
         // instance on Android. At least do whatever else we do on iOS.
         dataChannels.clear();
    }

    void createDataChannel(String label, ReadableMap config) {
        DataChannel.Init init = new DataChannel.Init();
        if (config != null) {
            if (config.hasKey("id")) {
                init.id = config.getInt("id");
            }
            if (config.hasKey("ordered")) {
                init.ordered = config.getBoolean("ordered");
            }
            if (config.hasKey("maxRetransmitTime")) {
                init.maxRetransmitTimeMs = config.getInt("maxRetransmitTime");
            }
            if (config.hasKey("maxRetransmits")) {
                init.maxRetransmits = config.getInt("maxRetransmits");
            }
            if (config.hasKey("protocol")) {
                init.protocol = config.getString("protocol");
            }
            if (config.hasKey("negotiated")) {
                init.negotiated = config.getBoolean("negotiated");
            }
        }
        DataChannel dataChannel = peerConnection.createDataChannel(label, init);
        // XXX RTP data channels are not defined by the WebRTC standard, have
        // been deprecated in Chromium, and Google have decided (in 2015) to no
        // longer support them (in the face of multiple reported issues of
        // breakages).
        int dataChannelId = init.id;
        if (-1 != dataChannelId) {
            dataChannels.put(dataChannelId, dataChannel);
            registerDataChannelObserver(dataChannelId, dataChannel);
        }
    }

    void dataChannelClose(int dataChannelId) {
        DataChannel dataChannel = dataChannels.get(dataChannelId);
        if (dataChannel != null) {
            dataChannel.close();
            dataChannels.remove(dataChannelId);
        } else {
            Log.d(TAG, "dataChannelClose() dataChannel is null");
        }
    }

    void dataChannelSend(int dataChannelId, String data, String type) {
        DataChannel dataChannel = dataChannels.get(dataChannelId);
        if (dataChannel != null) {
            byte[] byteArray;
            if (type.equals("text")) {
                try {
                    byteArray = data.getBytes("UTF-8");
                } catch (UnsupportedEncodingException e) {
                    Log.d(TAG, "Could not encode text string as UTF-8.");
                    return;
                }
            } else if (type.equals("binary")) {
                byteArray = Base64.decode(data, Base64.NO_WRAP);
            } else {
                Log.e(TAG, "Unsupported data type: " + type);
                return;
            }
            ByteBuffer byteBuffer = ByteBuffer.wrap(byteArray);
            DataChannel.Buffer buffer = new DataChannel.Buffer(byteBuffer, type.equals("binary"));
            dataChannel.send(buffer);
        } else {
            Log.d(TAG, "dataChannelSend() dataChannel is null");
        }
    }

    void getStats(String trackId, final Result promise) {
        MediaStreamTrack track = null;
        if (trackId == null
                || trackId.isEmpty()
                || (track = plugin.localTracks.get(trackId)) != null
                || (track = remoteTracks.get(trackId)) != null) {
            peerConnection.getStats(
                    new StatsObserver() {
                        @Override
                        public void onComplete(StatsReport[] reports) {
                            promise.success(statsToJSON(reports));
                        }
                    },
                    track);
        } else {
            Log.e(TAG, "peerConnectionGetStats() MediaStreamTrack not found for id: " + trackId);
        }
    }

    /**
     * Constructs a JSON <tt>String</tt> representation of a specific array of
     * <tt>StatsReport</tt>s (produced by {@link PeerConnection#getStats}).
     * <p>
     * On Android it is faster to (1) construct a single JSON <tt>String</tt>
     * representation of an array of <tt>StatsReport</tt>s and (2) have it pass
     * through the React Native bridge rather than the array of
     * <tt>StatsReport</tt>s.
     *
     * @param reports the array of <tt>StatsReport</tt>s to represent in JSON
     * format
     * @return a <tt>String</tt> which represents the specified <tt>reports</tt>
     * in JSON format
     */
    private String statsToJSON(StatsReport[] reports) {
        // If possible, reuse a single StringBuilder instance across multiple
        // getStats method calls in order to reduce the total number of
        // allocations.
        StringBuilder s = statsToJSONStringBuilder.get();
        if (s == null) {
            s = new StringBuilder();
            statsToJSONStringBuilder = new SoftReference(s);
        }

        s.append('[');
        final int reportCount = reports.length;
        for (int i = 0; i < reportCount; ++i) {
            StatsReport report = reports[i];
            if (i != 0) {
                s.append(',');
            }
            s.append("{\"id\":\"").append(report.id)
                .append("\",\"type\":\"").append(report.type)
                .append("\",\"timestamp\":").append(report.timestamp)
                .append(",\"values\":[");
            StatsReport.Value[] values = report.values;
            final int valueCount = values.length;
            for (int j = 0; j < valueCount; ++j) {
                StatsReport.Value v = values[j];
                if (j != 0) {
                    s.append(',');
                }
                s.append("{\"").append(v.name).append("\":\"").append(v.value)
                    .append("\"}");
            }
            s.append("]}");
        }
        s.append("]");

        String r = s.toString();
        // Prepare the StringBuilder instance for reuse (in order to reduce the
        // total number of allocations performed during multiple getStats method
        // calls).
        s.setLength(0);

        return r;
    }

    @Override
    public void onIceCandidate(final IceCandidate candidate) {
        Log.d(TAG, "onIceCandidate");
        WritableMap params = new JavaOnlyMap();
        params.putInt("id", id);
        WritableMap candidateParams = new JavaOnlyMap();
        candidateParams.putInt("sdpMLineIndex", candidate.sdpMLineIndex);
        candidateParams.putString("sdpMid", candidate.sdpMid);
        candidateParams.putString("candidate", candidate.sdp);
        params.putMap("candidate", candidateParams);

        plugin.sendEvent("peerConnectionGotICECandidate", params);
    }

    @Override
    public void onIceCandidatesRemoved(final IceCandidate[] candidates) {
        Log.d(TAG, "onIceCandidatesRemoved");
    }

    @Override
    public void onIceConnectionChange(PeerConnection.IceConnectionState iceConnectionState) {
        WritableMap params = new JavaOnlyMap();
        params.putInt("id", id);
        params.putString("iceConnectionState", iceConnectionStateString(iceConnectionState));

        plugin.sendEvent("peerConnectionIceConnectionChanged", params);
    }

    @Override
    public void onIceConnectionReceivingChange(boolean var1) {
    }

    @Override
    public void onIceGatheringChange(PeerConnection.IceGatheringState iceGatheringState) {
        Log.d(TAG, "onIceGatheringChange" + iceGatheringState.name());
        WritableMap params = new JavaOnlyMap();
        params.putInt("id", id);
        params.putString("iceGatheringState", iceGatheringStateString(iceGatheringState));
        plugin.sendEvent("peerConnectionIceGatheringChanged", params);
    }

    private String getReactTagForStream(MediaStream mediaStream) {
        for (Iterator<Map.Entry<String, MediaStream>> i
                    = remoteStreams.entrySet().iterator();
                i.hasNext();) {
            Map.Entry<String, MediaStream> e = i.next();
            if (e.getValue().equals(mediaStream)) {
                return e.getKey();
            }
        }
        return null;
    }

    @Override
    public void onAddStream(MediaStream mediaStream) {
        String streamReactTag = null;
        String streamId = mediaStream.label();
        // The native WebRTC implementation has a special concept of a default
        // MediaStream instance with the label default that the implementation
        // reuses.
        if ("default".equals(streamId)) {
            for (Map.Entry<String, MediaStream> e
                    : remoteStreams.entrySet()) {
                if (e.getValue().equals(mediaStream)) {
                    streamReactTag = e.getKey();
                    break;
                }
            }
        }

        if (streamReactTag == null){
            streamReactTag = plugin.getNextStreamUUID();
            remoteStreams.put(streamReactTag, mediaStream);
        }

        WritableMap params = new JavaOnlyMap();
        params.putInt("id", id);
        params.putString("streamId", streamId);
        params.putString("streamReactTag", streamReactTag);

        WritableArray tracks = new JavaOnlyArray();

        for (int i = 0; i < mediaStream.videoTracks.size(); i++) {
            VideoTrack track = mediaStream.videoTracks.get(i);
            String trackId = track.id();

            remoteTracks.put(trackId, track);

            WritableMap trackInfo = new JavaOnlyMap();
            trackInfo.putString("id", trackId);
            trackInfo.putString("label", "Video");
            trackInfo.putString("kind", track.kind());
            trackInfo.putBoolean("enabled", track.enabled());
            trackInfo.putString("readyState", track.state().toString());
            trackInfo.putBoolean("remote", true);
            tracks.pushMap(trackInfo);
        }
        for (int i = 0; i < mediaStream.audioTracks.size(); i++) {
            AudioTrack track = mediaStream.audioTracks.get(i);
            String trackId = track.id();

            remoteTracks.put(trackId, track);

            WritableMap trackInfo = new JavaOnlyMap();
            trackInfo.putString("id", trackId);
            trackInfo.putString("label", "Audio");
            trackInfo.putString("kind", track.kind());
            trackInfo.putBoolean("enabled", track.enabled());
            trackInfo.putString("readyState", track.state().toString());
            trackInfo.putBoolean("remote", true);
            tracks.pushMap(trackInfo);
        }
        params.putArray("tracks", tracks);

        plugin.sendEvent("peerConnectionAddedStream", params);
    }

    @Override
    public void onRemoveStream(MediaStream mediaStream) {
        String streamReactTag = getReactTagForStream(mediaStream);
        if (streamReactTag == null) {
            Log.w(TAG,
                "onRemoveStream - no remote stream for id: "
                    + mediaStream.label());
            return;
        }

        for (VideoTrack track : mediaStream.videoTracks) {
            this.remoteTracks.remove(track.id());
        }
        for (AudioTrack track : mediaStream.audioTracks) {
            this.remoteTracks.remove(track.id());
        }

        this.remoteStreams.remove(streamReactTag);

        WritableMap params = new JavaOnlyMap();
        params.putInt("id", id);
        params.putString("streamId", streamReactTag);
        plugin.sendEvent("peerConnectionRemovedStream", params);
    }

    @Override
    public void onAddTrack(MediaStream mediaStream,MediaStreamTrack track){
        Log.d(TAG, "onAddTrack");

        String streamId = mediaStream.label();
        String streamReactTag = streamId;

        WritableMap params = new JavaOnlyMap();
        params.putInt("id", id);
        params.putString("streamId", streamId);
        params.putString("streamReactTag", streamReactTag);

        String trackId = track.id();
        WritableMap trackInfo = new JavaOnlyMap();
        trackInfo.putString("id", trackId);
        trackInfo.putString("label", track.kind());
        trackInfo.putString("kind", track.kind());
        trackInfo.putBoolean("enabled", track.enabled());
        trackInfo.putString("readyState", track.state().toString());
        trackInfo.putBoolean("remote", true);

        params.putMap("track", trackInfo);

        plugin.sendEvent("peerConnectionAddedTrack", params);
    }

    @Override
    public void onRemoveTrack(MediaStream mediaStream,MediaStreamTrack track){
        Log.d(TAG, "onRemoveTrack");
        String streamId = mediaStream.label();
        String streamReactTag = streamId;
        WritableMap params = new JavaOnlyMap();
        params.putInt("id", id);
        params.putString("streamId", streamId);
        params.putString("streamReactTag", streamReactTag);

        String trackId = track.id();
        WritableMap trackInfo = new JavaOnlyMap();
        trackInfo.putString("id", trackId);
        trackInfo.putString("label", track.kind());
        trackInfo.putString("kind", track.kind());
        trackInfo.putBoolean("enabled", track.enabled());
        trackInfo.putString("readyState", track.state().toString());
        trackInfo.putBoolean("remote", true);

        params.putMap("track", trackInfo);

        plugin.sendEvent("peerConnectionRemovedTrack", params);
    }


    @Override
    public void onDataChannel(DataChannel dataChannel) {
        // XXX Unfortunately, the Java WebRTC API doesn't expose the id
        // of the underlying C++/native DataChannel (even though the
        // WebRTC standard defines the DataChannel.id property). As a
        // workaround, generated an id which will surely not clash with
        // the ids of the remotely-opened (and standard-compliant
        // locally-opened) DataChannels.
        int dataChannelId = -1;
        // The RTCDataChannel.id space is limited to unsigned short by
        // the standard:
        // https://www.w3.org/TR/webrtc/#dom-datachannel-id.
        // Additionally, 65535 is reserved due to SCTP INIT and
        // INIT-ACK chunks only allowing a maximum of 65535 streams to
        // be negotiated (as defined by the WebRTC Data Channel
        // Establishment Protocol).
        for (int i = 65536; i <= Integer.MAX_VALUE; ++i) {
            if (null == dataChannels.get(i, null)) {
                dataChannelId = i;
                break;
            }
        }
        if (-1 == dataChannelId) {
          return;
        }

        WritableMap dataChannelParams = new JavaOnlyMap();
        dataChannelParams.putInt("id", dataChannelId);
        dataChannelParams.putString("label", dataChannel.label());
        WritableMap params = new JavaOnlyMap();
        params.putInt("id", id);
        params.putMap("dataChannel", dataChannelParams);

        dataChannels.put(dataChannelId, dataChannel);
        registerDataChannelObserver(dataChannelId, dataChannel);

        plugin.sendEvent("peerConnectionDidOpenDataChannel", params);
    }

    private void registerDataChannelObserver(int dcId, DataChannel dataChannel) {
        // DataChannel.registerObserver implementation does not allow to
        // unregister, so the observer is registered here and is never
        // unregistered
        dataChannel.registerObserver(
            new DataChannelObserver(plugin, id, dcId, dataChannel));
    }

    @Override
    public void onRenegotiationNeeded() {
        WritableMap params = new JavaOnlyMap();
        params.putInt("id", id);
        plugin.sendEvent("peerConnectionOnRenegotiationNeeded", params);
    }

    @Override
    public void onSignalingChange(PeerConnection.SignalingState signalingState) {
        WritableMap params = new JavaOnlyMap();
        params.putInt("id", id);
        params.putString("signalingState", signalingStateString(signalingState));
        plugin.sendEvent("peerConnectionSignalingStateChanged", params);
    }

    @Override
    public void onAddRtpReceiver(final RtpReceiver receiver, final MediaStream[] mediaStreams) {
        Log.d(TAG, "onAddRtpReceiver");
    }

    @Nullable
    private String iceConnectionStateString(PeerConnection.IceConnectionState iceConnectionState) {
        switch (iceConnectionState) {
            case NEW:
                return "new";
            case CHECKING:
                return "checking";
            case CONNECTED:
                return "connected";
            case COMPLETED:
                return "completed";
            case FAILED:
                return "failed";
            case DISCONNECTED:
                return "disconnected";
            case CLOSED:
                return "closed";
        }
        return null;
    }

    @Nullable
    private String iceGatheringStateString(PeerConnection.IceGatheringState iceGatheringState) {
        switch (iceGatheringState) {
            case NEW:
                return "new";
            case GATHERING:
                return "gathering";
            case COMPLETE:
                return "complete";
        }
        return null;
    }

    @Nullable
    private String signalingStateString(PeerConnection.SignalingState signalingState) {
        switch (signalingState) {
            case STABLE:
                return "stable";
            case HAVE_LOCAL_OFFER:
                return "have-local-offer";
            case HAVE_LOCAL_PRANSWER:
                return "have-local-pranswer";
            case HAVE_REMOTE_OFFER:
                return "have-remote-offer";
            case HAVE_REMOTE_PRANSWER:
                return "have-remote-pranswer";
            case CLOSED:
                return "closed";
        }
        return null;
    }
}
