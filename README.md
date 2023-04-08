# ComputerVisionWithC
1. Local Filtering & Histogram
- read PGM file & smooth the image using a median filer and binomial filter
- compute the intensity histogram & Histogram Stretching & Histogram Equalization

2. Edge Detection
- compute the image gradients using the <b>Scharr operators</b>
- edge detection using the <b>Canny's Approach</b>

3. Image Segmentation
- Using K-means method

4. 3D Image Formation (3D -> 2D)
- read 3D file(.off)
- uv projection <img width="137" alt="스크린샷 2023-04-08 오후 7 32 09" src="https://user-images.githubusercontent.com/55435898/230716489-43393eb4-4b68-4e2f-8ed6-993d2f95b706.png">
- rigid transform <img width="148" alt="스크린샷 2023-04-08 오후 7 35 27" src="https://user-images.githubusercontent.com/55435898/230716605-6f169f30-f89e-4b5c-8b4d-e9097a2d866f.png">
- pinhole projection <img width="213" alt="스크린샷 2023-04-08 오후 7 36 18" src="https://user-images.githubusercontent.com/55435898/230716639-787671f9-a8ea-4b99-afbe-27ce9174cab6.png">
- display output with a ppm file
