# -*- coding: utf-8 -*-
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Coordinate transformation to inertial
def CT_e2I(tx,ty,tz):
    # Must be in radians
    # Converts from actual orientation to intertial frame, used to add displacement
    # data to the rest of the displacement stuff 

    Ce2I = np.array([[np.cos(ty)*np.cos(tz),np.cos(ty)*np.sin(ty),-np.sin(ty)],
                     [np.sin(tx)*np.sin(ty)*np.cos(tz)-np.cos(tx)*np.sin(tz),
                      np.sin(tx)*np.sin(ty)*np.sin(tz)+np.cos(tx)*np.cos(tz),
                      np.sin(tx)*np.cos(ty)],
                     [np.cos(tx)*np.sin(ty)*np.cos(tz)+np.sin(tx)*np.sin(tz),
                      np.cos(tx)*np.sin(ty)*np.sin(tz)-np.sin(tx)*np.cos(tz),
                      np.cos(tx)*np.cos(ty)]])
    
    return Ce2I

# Load data into array
df = pd.read_csv ('Book1.csv', header=None)
# print(df)
data = df.to_numpy()
# print(data.shape)

# Find gravity direction

# Interpolation
subdivisions = 10
N = data.shape[0]
expanded = np.empty(((N-1)*10 + 1, 7))
for sample in range(N - 1):
    step = (data[sample+1] - data[sample]) / (subdivisions)
    for i in range(subdivisions):
        # print(data[sample] + step * i)
        expanded[sample*subdivisions + i] = data[sample] + step * i
expanded[(N-1)*subdivisions] = data[-1]

np.set_printoptions(suppress=True)
# print(expanded)
data = expanded

# Displacement calculations
plot_position = []
plot_y = []
position = np.zeros(3)
angle = np.zeros(3)
N = data.shape[0]
for sample in range(1, N):
    dt = data[sample,0]
    
    accel = np.array([[data[sample-1,1], data[sample-1,2], data[sample-1,3]],
                      [data[sample,1], data[sample,2], data[sample,3]]])
    gyro = np.array([[data[sample-1,4], data[sample-1,5], data[sample-1,6]],
                    [data[sample,4], data[sample,5], data[sample,6]]])
    
    tx = np.trapz([gyro[0,0],gyro[1,0]],dx=dt)
    ty = np.trapz([gyro[0,1],gyro[1,1]],dx=dt)
    tz = np.trapz([gyro[0,2],gyro[1,2]],dx=dt)
    
    dx = np.trapz([accel[0,0],accel[1,0]],dx=dt)
    dy = np.trapz([accel[0,1],accel[1,1]],dx=dt)
    dz = np.trapz([accel[0,2],accel[1,2]],dx=dt)
    
    dpe = np.array([[dx],[dy],[dz]])

    
    Ce2I = CT_e2I(angle[0] + tx, angle[1] + ty, angle[2] + tz)
    
    dpI = Ce2I @ dpe
    
    angle = angle + np.array([tx,ty,tz])
    
    position = position + dpI.T

    plot_position.append(position[0][0])
    plot_y.append(position[0][1])

# NOT DISPLACEMENT WE WANT FOR FINAL POSITION, THIS IS TOTAL DISPLACEMENT INCLUDING
# ELEVATION
displacement = np.linalg.norm(position)

plt.figure(figsize=(10,8))
plt.plot(plot_position)
plt.show()

plt.figure(figsize=(10,10))
plt.plot(plot_position, plot_y)