import argparse
import time

import cv2
import matplotlib.cm as cm
import numpy as np

from pyphoxi import PhoXiSensor


# https://stackoverflow.com/questions/33322488/how-to-change-image-illumination-in-opencv-python
def adjust_gamma(image, gamma=1.0):
    invGamma = 1.0 / gamma
    table = np.array([((i / 255.0) ** invGamma) * 255 for i in np.arange(0, 256)]).astype("uint8")
    return cv2.LUT(image, table)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Capture PhoXi Sensor Data')
    parser.add_argument('tcp_ip', type=str, help="IP address of TCP server.")
    parser.add_argument('tcp_port', type=int, help="Port number of TCP server.")
    args, unparsed = parser.parse_known_args()

    camera = PhoXiSensor(args.tcp_ip, args.tcp_port)
    camera.start()
    time.sleep(1)
    print("[!] Successfully connected to camera.")

    print("[*] Intrinsics: {}".format(camera.intrinsics))
    print("[*] Distortion: {}".format(camera.distortion))

    current_milli_time = lambda: int(round(time.time() * 1000))
    cv2.namedWindow("Phoxi Camera Window")
    frame_id_prev = 0
    timestamp_prev = current_milli_time()
    fps = 1
    while True:
        frame_id, gray_im, depth_im = camera.get_frame(True)
        timestamp_curr = current_milli_time()
        if timestamp_curr - timestamp_prev > 1000:
            fps = 1000 * (frame_id - frame_id_prev) / (timestamp_curr - timestamp_prev)
            timestamp_prev = timestamp_curr
            frame_id_prev = frame_id
        color_im_cm = np.repeat(gray_im[..., np.newaxis], 3, axis=-1)
        depth_im_cm = (depth_im - depth_im.min()) / (depth_im.max() - depth_im.min())
        depth_im_cm = cm.viridis(depth_im_cm, bytes=True)[..., :3]
        images = np.hstack([adjust_gamma(color_im_cm, 2.0), depth_im_cm])
        cv2.putText(
            images,
            "fps: {:.2f}".format(fps),
            (10, 30),
            cv2.FONT_HERSHEY_SIMPLEX,
            1,
            (255, 0, 0),
            2
        )
        cv2.putText(
            images,
            "id: {}".format(frame_id),
            (10, 60),
            cv2.FONT_HERSHEY_SIMPLEX,
            1,
            (255, 0, 0),
            2
        )
        cv2.imshow("color", images)
        if cv2.waitKey(1) == ord("q"):
            camera.stop()
            break