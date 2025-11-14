import threading, socket, struct, numpy as np

from socketstream import SocketStream
from bytestream import ByteStream

try:
    import bpy
except:
    bpy = None

scale = 0.01
depth_scale = 20.0

def quad(width, column, row):
    return (column * width + row,
            (column + 1) * width + row,
            (column + 1) * width + 1 + row,
            column * width + 1 + row)


def to_mesh(c_width, c_height, color_frame, d_width, d_height, depth_frame):
    color_data = np.frombuffer(color_frame, dtype=np.uint8, count=c_width * c_height * 4)
    color_data = color_data.reshape((c_width, c_height, 4)) / 255
    color_data[::, ::, 3] = 1
    color_data[::, ::, [0, 2]] = color_data[::, ::, [2, 0]]

    depth_data = (- (np.frombuffer(depth_frame, dtype=np.uint16, count=d_width * d_height) / 0xFF_FF)).reshape((d_width, d_height))

    image = bpy.data.images.get("KINECT_COLOR") or bpy.data.images.new(
        "KINECT_COLOR",
        width=c_width,
        height=c_height)
    image.pixels = color_data[::-1, ::-1, ::].flatten()

    mesh_obj = bpy.data.objects.get('KINECT_MESH')

    if mesh_obj is None:
        mesh_data = bpy.data.meshes.new("KINECT_MESH_DATA")
        mesh_obj = bpy.data.objects.new("KINECT_MESH", mesh_data)
        bpy.context.scene.collection.objects.link(mesh_obj)
        # Construct the quads
        verts = np.array([((d_height - x) * scale, (d_width - y) * scale, depth_data[y, x] * depth_scale) for x in range(d_height) for y in range(d_width)], dtype=np.float)
        faces = [quad(d_width, x, y) for x in range(d_height - 1) for y in range(d_width - 1)]
        mesh_data.from_pydata(verts, [], faces)
        mesh_obj.data.update()
    print(len(mesh_obj.data.vertices), depth_data.shape)
    # Update the position of the quad vertices
    vertices = np.zeros((len(mesh_obj.data.vertices) * 3,), dtype=np.float32)
    mesh_obj.data.vertices.foreach_get('co', vertices)
    vertices[2::3] = depth_data.reshape(-1)
    mesh_obj.data.vertices.foreach_set('co', list(vertices))
    mesh_obj.data.update()


def thread_func():
    print("Starting KineticScan")
    server_socket = socket.socket(socket.AF_INET,
                                  socket.SOCK_STREAM,
                                  socket.IPPROTO_TCP)
    try:
        server_socket.bind(('127.0.0.1', 13279))
    except Exception as e:
        print("Failed to bind port:", e)
    else:
        try:
            print("Listening for client")
            server_socket.listen(5)
            (client_socket, address) = server_socket.accept()
            print("Found client ", address)
            my_socket = MySocket(client_socket)
            try:
                while True:
                    colour_width = my_socket.read_u32()
                    colour_height = my_socket.read_u32()
                    colour_frame = my_socket.read(
                        colour_width * colour_height * 4)
                    depth_width = my_socket.read_u32()
                    depth_height = my_socket.read_u32()
                    depth_frame = my_socket.read(
                        depth_width * depth_height * 2)
                    try:
                        to_mesh(colour_width, colour_height, colour_frame,
                                depth_width, depth_height, depth_frame)
                    except Exception as e:
                        print("Failed to write to mesh:", e)
            except Exception as e:
                print(e)
                print("Client probably disconnected")
            client_socket.shutdown(socket.SHUT_RDWR)
            client_socket.close()
        except Exception as e:
            print(e)
    server_socket.close()
    print("KineticScan finished")


if __name__ == '__main__':
    thread = threading.Thread(target=thread_func)
    thread.start()