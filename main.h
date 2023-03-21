/*
 * main.h
 *
 *  Created on: 20-Mar-2023
 *      Author: ee212778
 */

#include <gst/gst.h>

#ifndef MAIN_H_
#define MAIN_H_

typedef struct elemdata {

	GstElement* pipeline;
	GstElement* filenfo;
	GstElement* filesrc1;
	GstElement* aud_decodebin;
	GstElement* vid_decodebin;
	GstElement* image_freez;
	GstElement* filesrc2;

	struct vid_data {
		GstElement* queue;
		GstElement* videoconvert;
		GstElement* vd_enc;
	};

	struct vid_data vi_data;

	struct aud_data {
		GstElement* tee;
		GstElement* queue1;
		GstElement* audioconvert;
		GstElement* audioresample;
		GstElement* progressreport;
		GstElement* vorbisenc;

		GstElement* queue2;
	};


	struct aud_data aud_data;

	GstElement* muxer;
	GstElement* filesink;
	GstElement* fakesink;

} ElemData;


typedef struct applicatElem{
	GstElement *pipeline;
	GstBus* bus;
}EssenElem;


#endif /* MAIN_H_ */
