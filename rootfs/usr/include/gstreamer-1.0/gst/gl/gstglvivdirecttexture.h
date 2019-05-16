/*
 * GStreamer
 * Copyright (c) 2015, Freescale Semiconductor, Inc. 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GST_GL_VIVDIRECT_H__
#define __GST_GL_VIVDIRECT_H__

#include <gst/video/video.h>
#include <gst/gl/gstgl_fwd.h>

G_BEGIN_DECLS

gboolean gst_is_physical_buffer (GstBuffer *buffer);
gboolean gst_gl_viv_direct_bind_gstbuffer (GstGLContext * context, guint tex_id, GstVideoInfo * info, GstBuffer * buffer)

G_END_DECLS

#endif /* __GST_GL_VIVDIRECT_H__ */
