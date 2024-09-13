import serial
import math
import time
import numpy as np
import open3d as o3d
 
s = serial.Serial('COM4', 115200, timeout=25)
 
print("Opening: " + s.name)
 
s.reset_output_buffer()
s.reset_input_buffer()
 
input("Press Enter to start communication...")
 
myfile = open("hallway_final2.xyz", "w")
 
# Define the number of measurements per layer and the total number of layers
measurements_per_layer = 32
total_layers = 3
distance_between_layers = 100
 
for layer in range(total_layers):
    for measurement in range(measurements_per_layer):
        data = s.readline()
        value = data.decode().strip()  # Remove the newline at the end
        # It is important to validate the received data before processing
        try:
            distance = int(value)
        except ValueError:
            print(f"Invalid data received: {value}")
            continue  # Skip to the next iteration
 
        # Calculate the angle in radians
        angle_rad = 2 * math.pi * measurement / measurements_per_layer
        y = distance * math.cos(angle_rad)
        z = distance * math.sin(angle_rad)
        x = layer * distance_between_layers  # x-coordinate is the layer index times the distance between layers
 
        # Write the calculated point to the file
        myfile.write(f"{x} {y} {z}\n")
 
        # Display the value in the console
        print(f"Layer {layer}, Measurement {measurement}: {value}")
 
    time.sleep(25)
 
myfile.close()
 
print("Closing: " + s.name)
s.close()

def create_lines(vertices, measurements_per_layer):
    lines = []
    num_vertices = len(vertices)
    layers = num_vertices // measurements_per_layer
    # Connect points within layers
    for layer in range(layers):
        for i in range(measurements_per_layer):
            idx = layer * measurements_per_layer + i
            next_idx = idx + 1 if (i + 1) < measurements_per_layer else layer * measurements_per_layer
            lines.append([idx, next_idx])
    # Connect corresponding points in consecutive layers
    for i in range(measurements_per_layer):
        for layer in range(layers - 1):
            idx = layer * measurements_per_layer + i
            upper_idx = idx + measurements_per_layer
            lines.append([idx, upper_idx])
    return lines
 
if __name__ == "__main__":
    # Read the test data from the file we created
    print("Read in the prism point cloud data (pcd)")
    pcd = o3d.io.read_point_cloud("hallway_final2.xyz", format="xyz")
    # Visualize the point cloud data
    print("Let's visualize the PCD: (spawns separate interactive window)")
    o3d.visualization.draw_geometries([pcd])
    # Extract vertices
    vertices = np.asarray(pcd.points)
    measurements_per_layer = 32  # Define how many measurements you have per layer
    lines = create_lines(vertices, measurements_per_layer)
    # Create a line set
    line_set = o3d.geometry.LineSet(
        points=o3d.utility.Vector3dVector(vertices),
        lines=o3d.utility.Vector2iVector(lines)
    )
    # Visualize the line set
    print("Visualizing the line set:")
    o3d.visualization.draw_geometries([line_set])
