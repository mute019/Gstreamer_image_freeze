/*
 * main.cpp
 *
 *  Created on: 20-Mar-2023
 *      Author: ee212778
 */

#include <iostream>
#include <gstreamer-1.0/gst/gst.h>
#include <string>
#include<unistd.h>

#include "main.h"

static void pad_added_handler(GstElement *, GstPad* , ElemData* );
static GstPadProbeReturn eos_probe_cb (GstPad * , GstPadProbeInfo * , gpointer );


int exer3() {

	GstBus* bus;
	GstMessage* message;

	GstStateChangeReturn ret;
	EssenElem* req_ele = new EssenElem;
	gboolean terminate = FALSE;
	ElemData data;

	gst_init(nullptr, nullptr);


	data.filesrc1 = gst_element_factory_make("filesrc", "source1");

	data.aud_decodebin = gst_element_factory_make("decodebin", "aud_decoder");
	data.vid_decodebin = gst_element_factory_make("decodebin", "vid_decoder");
	data.filesrc2 = gst_element_factory_make("filesrc", "source2");
	data.image_freez = gst_element_factory_make("imagefreeze", "img_freez");

	// audio source

	data.aud_data.tee = gst_element_factory_make("tee", "tee");

	data.aud_data.queue1 = gst_element_factory_make("queue", "audi_queue1");
	data.aud_data.queue2 = gst_element_factory_make("queue", "audi_queue2");
	data.aud_data.audioconvert = gst_element_factory_make("audioconvert", "audi_convert");
	data.aud_data.audioresample = gst_element_factory_make("audioresample", "audi_resample");
	data.aud_data.progressreport = gst_element_factory_make("progressreport", "progress");
	data.aud_data.vorbisenc = gst_element_factory_make("vorbisenc", "aud_enc");


	// video sources

	data.vi_data.queue = gst_element_factory_make("queue", "vid_queue");
	data.vi_data.videoconvert = gst_element_factory_make("videoconvert", "vid_convert");
	data.vi_data.vd_enc = gst_element_factory_make("vp8enc", "vd_enc");



	data.muxer = gst_element_factory_make("webmmux", "mux");
	data.filesink = gst_element_factory_make("filesink", "sink");
	data.fakesink = gst_element_factory_make("fakesink", "fsink");

	data.pipeline = gst_pipeline_new("test-pipeline");

	if (!data.pipeline || !data.filesrc1 || !data.aud_decodebin || !data.vid_decodebin || !data.filesrc2 || !data.image_freez || !data.muxer || !data.filesink || !data.fakesink || !data.aud_data.tee || !data.aud_data.queue2|| !data.aud_data.queue1 || !data.vi_data.queue || !data.aud_data.audioconvert || !data.aud_data.audioresample || !data.aud_data.progressreport || !data.aud_data.vorbisenc || !data.vi_data.videoconvert || !data.vi_data.vd_enc) {
		g_printerr("Not all elements can be created!");
		return -1;
	}

	gst_bin_add_many(GST_BIN (data.pipeline), data.filesrc1, data.aud_decodebin, data.vid_decodebin, data.filesrc2, data.image_freez, data.muxer, data.filesink, data.fakesink, data.aud_data.tee, data.aud_data.queue1, data.aud_data.queue2, data.aud_data.audioconvert, data.aud_data.audioresample, data.aud_data.progressreport, data.aud_data.vorbisenc, data.vi_data.queue, data.vi_data.videoconvert, data.vi_data.vd_enc, NULL);

	if (!gst_element_link_many(data.filesrc2, data.aud_decodebin, NULL)) {
		g_printerr("Audio src elements could not be linked!");
		gst_object_unref(data.pipeline);
		return -1;
	}


	if (!gst_element_link_many(data.filesrc1, data.vid_decodebin, NULL)) {
		g_printerr("Image src elements could not be linked!");
		gst_object_unref(data.pipeline);
		return -1;
	}

	if (!gst_element_link_many(data.aud_data.queue2, data.fakesink, NULL)) {
		g_printerr("Fakesink elements could not be linked!");
		gst_object_unref(data.pipeline);
		return -1;
	}

	if (!gst_element_link_many(data.muxer, data.filesink, NULL)) {
		g_printerr("Muxer elements could not be linked!");
		gst_object_unref(data.pipeline);
		return -1;
	}


	if (!gst_element_link_many(data.image_freez, data.vi_data.queue, data.vi_data.videoconvert, data.vi_data.vd_enc, data.muxer, NULL)) {
		g_printerr("Video elements could not be linked!");
		gst_object_unref(data.pipeline);
		return -1;
	}

	if (!gst_element_link_many(data.aud_data.queue1 ,data.aud_data.audioconvert, data.aud_data.audioresample, data.aud_data.progressreport, data.aud_data.vorbisenc, data.muxer, NULL)) {
		g_printerr("Audio elements could not be linked!");
		gst_object_unref(data.pipeline);
		return -1;
	}


	g_signal_connect(data.aud_decodebin, "pad-added", G_CALLBACK (pad_added_handler), &data);
	g_signal_connect(data.vid_decodebin, "pad-added", G_CALLBACK (pad_added_handler), &data);

	GstPad* aud1_pad = gst_element_get_request_pad(data.aud_data.tee, "src_%u");
	GstPad* aud2_pad = gst_element_get_request_pad(data.aud_data.tee, "src_%u");

	GstPad* que1_pad = gst_element_get_static_pad(data.aud_data.queue1, "sink");
	GstPad* que2_pad = gst_element_get_static_pad(data.aud_data.queue2, "sink");


	if (
			gst_pad_link (aud1_pad, que1_pad) != GST_PAD_LINK_OK ||
			gst_pad_link (aud2_pad, que2_pad) != GST_PAD_LINK_OK

	) {

		g_printerr("Audio tee pads failed to link! \n");

	}


	//g_object_set(G_OBJECT(data.image_freez), "num-buffers", 1000, NULL);

	// file://<path>
	g_object_set(G_OBJECT(data.filesrc1), "location", "/home/ee212778/Downloads/sample_img.jpg", NULL);
	g_object_set(G_OBJECT(data.filesrc2), "location", "/home/ee212778/Downloads/AUD-20230320-WA0000.mp3", NULL);
	g_object_set(G_OBJECT(data.filesink), "location", "/home/ee212778/Downloads/thumb_sample.webm", NULL);



	ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);

	if (ret == GST_STATE_CHANGE_FAILURE) {
	    g_printerr ("Unable to set the pipeline to the playing state.\n");
	    gst_object_unref (data.pipeline);
	    return -1;
	}


	bus = gst_element_get_bus(data.pipeline);

	req_ele->bus = bus;
	req_ele->pipeline = data.pipeline;
	gst_pad_add_probe (que2_pad, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM, eos_probe_cb, req_ele, NULL);



	do {
		message = gst_bus_timed_pop_filtered(
				bus,
				GST_CLOCK_TIME_NONE,
				(GstMessageType) (GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

		if (message != nullptr) {

			switch(GST_MESSAGE_TYPE(message)) {
				case GST_MESSAGE_ERROR:
					GError* gerror;
					gchar* debg_info;
					gst_message_parse_error(message, &gerror, &debg_info);

					g_printerr("Error received from the element %s : %s \n", GST_OBJECT_NAME(message->src), gerror->message);
					g_printerr("Debugging information: %s \n", debg_info? debg_info : "none");

					g_clear_error(&gerror);
					g_free(debg_info);
					break;
				case GST_MESSAGE_EOS:

					g_print("Stream ended! \n");
					terminate = TRUE;
					break;
				case GST_MESSAGE_STATE_CHANGED:
					if (GST_MESSAGE_SRC(message) == GST_OBJECT(data.pipeline)) {
						GstState oldstate, newstate, pendingstate;
						gst_message_parse_state_changed(message, &oldstate, &newstate, &pendingstate);
						g_print ("Pipeline state changed from %s to %s:\n",
										gst_element_state_get_name (oldstate), gst_element_state_get_name (newstate));

						if (newstate == GST_STATE_NULL) {
							//terminate = TRUE;
							gst_element_unlink(data.image_freez, data.aud_data.queue1);
							gst_element_set_state(data.image_freez, GST_STATE_NULL);
							gst_bin_remove(GST_BIN(data.pipeline), data.image_freez);
							gst_object_unref(data.image_freez);

							gst_element_send_event(data.pipeline, gst_event_new_eos());
						}
					}

					break;
				default:
					g_printerr("Unexpected Error!!\n");
					break;

			}


			gst_message_unref(message);

		}


	} while(!terminate);


	gst_object_unref(bus);
	gst_element_set_state (data.pipeline, GST_STATE_NULL);
	gst_object_unref (data.pipeline);

	return 0;
}


static void pad_added_handler(GstElement *src, GstPad* new_pad, ElemData* data) { // @suppress("Unused static function")
	GstPadLinkReturn ret;

	GstCaps *new_pad_cap = NULL;
	GstStructure *new_pad_struct = NULL;
	const gchar *new_pad_type = NULL;

	gboolean clearence = FALSE;

	g_print("Received new pad '%s' from '%s':\n", GST_PAD_NAME(new_pad), GST_ELEMENT_NAME(src));

	new_pad_cap = gst_pad_get_current_caps (new_pad);
	new_pad_struct = gst_caps_get_structure (new_pad_cap, 0);
	new_pad_type = gst_structure_get_name (new_pad_struct);

	GstPad* sink_pad;

	if (g_str_has_prefix (new_pad_type, "video/x-raw")) {
		sink_pad = gst_element_get_static_pad(data->image_freez, "sink");
		g_print ("It has type '%s'.\n", new_pad_type);

	}

	else if (g_str_has_prefix (new_pad_type, "audio/x-raw")) {
		sink_pad = gst_element_get_static_pad(data->aud_data.tee, "sink");
		g_print ("It has type '%s'.\n", new_pad_type);
	}


	else {
		g_print ("It has type '%s' which is not raw audio/video. Ignoring.\n", new_pad_type);
		clearence = TRUE;
	}

	ret = gst_pad_link (new_pad, sink_pad);
	if (GST_PAD_LINK_FAILED (ret)) {
		g_print ("Type is '%s' but link failed.\n", new_pad_type);

	} else {
		g_print ("Link succeeded (type '%s').\n", new_pad_type);
	}

	if (clearence) {
		if (new_pad_cap != NULL)
			gst_caps_unref (new_pad_cap);

		gst_object_unref (sink_pad);
	}

	return;

}

static GstPadProbeReturn eos_probe_cb (GstPad * pad, GstPadProbeInfo * info, gpointer user_data){
	EssenElem *pipelinedata = (EssenElem *)user_data;

	if (GST_EVENT_TYPE (GST_PAD_PROBE_INFO_DATA (info)) == GST_EVENT_EOS) {
		//printf ("EOS detected\n");
		sleep(6);
	// Stop the pipeline when EOS is detected

		//gst_object_unref(src);
		//gst_object_unref(dest);

//		GstMessage *message = gst_message_new_state_changed(
//		    GST_OBJECT(pipelinedata->pipeline), GST_STATE_PLAYING, GST_STATE_NULL, GST_STATE_VOID_PENDING);

		GstMessage* message = gst_message_new_eos(GST_OBJECT(pipelinedata->pipeline));

		gst_bus_post(pipelinedata->bus, message);

		g_print("EOS-Sent!\n");

	// Remove the probe
		gst_pad_remove_probe (pad, GST_PAD_PROBE_INFO_ID (info));

		return GST_PAD_PROBE_REMOVE;
	}

	return GST_PAD_PROBE_OK;
}



int main() {
	exer3();
}



