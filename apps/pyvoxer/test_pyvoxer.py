from pyvoxer import *

dataset=Dataset.Load("C:/Users/wyz/projects/voxer/test_data/foot_256_256_256_uint8.raw")

tfcn = TransferFunction()
tfcn.add_point(0.0,0.0,[0.0,0.0,0.0])
tfcn.add_point(0.5,0.0,[1.0,1.0,0.0])
tfcn.add_point(1.0,1.0,[1.0,1.0,1.0])
volume=Volume()
volume.set_dataset(dataset)
volume.set_transfer_function(tfcn)

camera=Camera()
# camera.zoom=60.0
camera.width=1200
camera.height=900
# camera.up=[0.0,0.0,1.0]
camera.pos=[256,256,256]
camera.target=[0,0,0]



renderer = VolumeRenderer('opengl')

renderer.set_camera(camera)
renderer.add_volume(volume)

renderer.render()
image=renderer.get_colors()

encoded=Image.encode(image,Image.Format.JPEG,Image.Quality.HIGH,True)
f=open("result.jpeg","wb")
f.write(bytearray(encoded.data))
