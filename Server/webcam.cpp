//#include <opencv2/opencv.hpp>
//#include <iostream>
//
//using namespace cv;
//using namespace std;
//
//// Hàm kiểm tra xem webcam có đang bật hay không
//bool isWebcamActive(VideoCapture& cap) {
//    return cap.isOpened();
//}
//
//int main() {
//    VideoCapture cap(0); // Mở webcam mặc định (ID = 0)
//
//    if (isWebcamActive(cap)) {
//        cout << "Webcam đang bật. Tắt webcam..." << endl;
//        cap.release(); // Tắt webcam
//    }
//    else {
//        cout << "Webcam đang tắt. Bật webcam và chụp hình..." << endl;
//
//        // Bật lại webcam
//        cap.open(0);
//        if (!cap.isOpened()) {
//            cerr << "Không thể mở webcam!" << endl;
//            return -1;
//        }
//
//        // Đợi một chút để webcam ổn định
//        this_thread::sleep_for(chrono::seconds(1));
//
//        // Chụp ảnh từ webcam
//        Mat frame;
//        cap >> frame; // Lấy một khung hình từ webcam
//
//        if (frame.empty()) {
//            cerr << "Không thể chụp ảnh từ webcam!" << endl;
//            return -1;
//        }
//
//        // Lưu ảnh ra file
//        string filename = "captured_image.jpg";
//        imwrite(filename, frame);
//        cout << "Ảnh đã được chụp và lưu tại: " << filename << endl;
//    }
//
//    cap.release(); // Giải phóng webcam
//    destroyAllWindows(); // Đóng các cửa sổ hiển thị (nếu có)
//
//    return 0;
//}
