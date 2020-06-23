# MIT License

# Copyright (c) 2020 REDxEYE

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import struct

import numpy as np

try:
    import bpy
except:
    bpy = None

scale = 0.01


def quad(width, column, row):
    return (column * width + row,
            (column + 1) * width + row,
            (column + 1) * width + 1 + row,
            column * width + 1 + row)


def to_mesh(color_frame, depth_frame):
    c_width, c_height = struct.unpack_from('ii', color_frame, 0)
    d_width, d_height = struct.unpack_from('ii', depth_frame, 0)  # 640 480
    color_data = np.frombuffer(color_frame[8:], dtype=np.uint8, count=c_width * c_height * 4)
    color_data = color_data.reshape((c_width, c_height, 4)) / 255
    depth_data = 1 - (np.frombuffer(depth_frame[8:], dtype=np.uint16, count=d_width * d_height) / 0xFF_FF)

    color_data[::, ::, 3] = 1

    image = bpy.data.images.get("KINECT_COLOR") or bpy.data.images.new(
        "KINECT_COLOR",
        width=c_width,
        height=c_height)
    image.pixels = color_data[::-1, ::-1, ::].flatten()
    if bpy.data.objects.get('KINECT_MESH') is None:
        mesh_data = bpy.data.meshes.new("KINECT_MESH_DATA")
        mesh_obj = bpy.data.objects.new("KINECT_MESH", mesh_data)
        bpy.context.scene.collection.objects.link(mesh_obj)
        verts = np.array([(x * scale, y * scale, 0) for x in range(d_height) for y in range(d_width)], dtype=np.float)
        faces = [quad(d_width, x, y) for x in range(d_height - 1) for y in range(d_width - 1)]

        mesh_data.from_pydata(verts, [], faces)
    else:
        mesh_obj = bpy.data.objects.get("KINECT_MESH")
        mesh_data = mesh_obj.data
    print(len(mesh_data.vertices), depth_data.shape)
    vertices = np.zeros((len(mesh_data.vertices) * 3,), dtype=np.float32)
    mesh_data.vertices.foreach_get('co',vertices)
    vertices[0::3]=depth_data.reshape(-1)
    mesh_data.vertices.foreach_set('co', list(vertices))


if __name__ == '__main__':
    with open(r'F:\PYTHON_STUFF\Kinect2Blender\test_data\colour.frame', 'rb') as f:
        c_frame = f.read(-1)
    with open(r'F:\PYTHON_STUFF\Kinect2Blender\test_data\depth.frame', 'rb') as f:
        d_frame = f.read(-1)
    to_mesh(c_frame, d_frame)
