%{ 
detect_parking_spaces.m file
Ryan Mark
Final Masters Project
%}



clc;
clear all;
close all;

%% Loading Images into MATLAB
filledParking = imread("cars2.jpg");
emptyParking = imread("empty2.jpg");
parkingMask = imread("mask2.jpg");

%% Resizing for Zybo
filledParking = imresize(filledParking, [452 640]);
emptyParking = imresize(emptyParking, [452 640]);
parkingMask = imresize(parkingMask, [452 640]);
%% record intial input image
imwrite(filledParking, 'colored_car.bmp');

%% Pre-Processing of Images

% Display filled parking image
fig1 = figure();
fig1.WindowState = 'maximized';
imshow(filledParking);
axis off;
title('Parking filled with cars');

% Display empty parking image
fig2 = figure();
fig2.WindowState = 'maximized';
imshow(emptyParking);
axis off;
title('Empty Parking');

% Display binary parking mask
fig3 = figure();
fig3.WindowState = 'maximized';
bwMask = max(parkingMask, [], 3) == 255;  % masks images at maximum intensity of 255
x = bwMask;
se1 = strel('rectangle', [12, 8]);        % structuring element
bwMask = imclose(bwMask, se1);           % morphological closing to fill holes to clean mask
imwrite(bwMask, 'final_parkmask.bmp');
props = regionprops(bwMask, 'BoundingBox', 'Area', 'Centroid'); % detects object's BoundingBox, Area, and Centroid

imshow(bwMask);                           % display binary mask 
m = 1;                                    % variable to count and store detected objects (change to 0 when rewriting in C)
for i = 1:length(props)
    if props(i).Area > 1000               % check if area of detected object is higher than 1000 to remove noise
        allLocations{m} = round(props(i).BoundingBox);  % save bounding box locations of detected objects
        allCentroids{m} = round(props(i).Centroid);     % save centroid locations of detected objects
        m = m + 1;
    end
end
axis off;
title('Parking Mask');

% Create difference image using fixed-point representation
fig4 = figure();
fig4.WindowState = 'maximized';
T = numerictype(1, 16, 0);                 % signed, 16 bits, 0 fraction bits
A = fi(filledParking, T);
B = fi(emptyParking, T);
diffImage = abs(A - B);
diffImage = cast(diffImage, "like", A);
diffImage = uint8(diffImage);
imshow(diffImage);
axis off;
title('Difference Image');

% Convert difference image to grayscale and mask it
fig5 = figure();
fig5.WindowState = 'maximized';
diffImage = rgb2gray(diffImage);          % grayscale conversion
diffImage(~bwMask) = 0;                   % set all pixels outside the mask to 0
imshow(diffImage);
axis off;
title('Gray Scale Difference Image');
grayI = diffImage;
imwrite(grayI, "final_graycars.jpg");

% Apply threshold and morphological operations
threshold = graythresh(diffImage) * 100;  % threshold value using Otsu’s method
parkedCars = diffImage > threshold;       % apply threshold
fig6 = figure();
fig6.WindowState = 'maximized';
imshow(parkedCars);
axis off;
title('Thresholded image');
se = ones(25, 10);   % structuring element

%% my morphological operation function
parkedCars = Morph_Cars(parkedCars,se);

%% morphlogical operation using the built in functions
%parkedCars = imclose(parkedCars, se);     % fill the holes
%parkedCars = imopen(parkedCars, se);      % erode the image to clean small noise

imwrite(parkedCars, 'final_carmask.bmp');

% Display final binary mask of parked cars
fig7 = figure();
fig7.WindowState = 'maximized';
imshow(parkedCars);
axis off;
title(sprintf('Parked Cars Binary Image with Threshold = %.1f', threshold));

% Analyze parked car mask and mark detected cars
iprops = regionprops(bwMask, parkedCars, 'BoundingBox', 'MeanIntensity'); % detect Bounding Box and Mean Intensity
m = 1;
for i = 1:length(iprops)
    if iprops(i).MeanIntensity > 0.1      % check if mean intensity > 0.1
        rectangle('Position', iprops(i).BoundingBox, 'EdgeColor', 'y', 'LineWidth', 2); % yellow box around detected car
        takenLocations{m} = iprops(i).BoundingBox; % save bounding box info
        m = m + 1;
    end
end

% Final result visualization with green O and red X
fig8 = figure();
imshow(filledParking);
fig8.WindowState = 'maximized';

for i = 1:length(allLocations)
    taken = false;
    box1 = allLocations{i};
    r1 = rectangle('Position', box1, 'EdgeColor', 'r', 'LineWidth', 2); % red box for all spots

    for j = 1:length(takenLocations)
        box2 = takenLocations{j};
        r2 = rectangle('Position', box2, 'EdgeColor', 'y', 'LineWidth', 2); % yellow box for detected cars
        ratio = bboxOverlapRatio(box1, box2); % check if boxes overlap
        if ratio > 0.1
            cent = allCentroids{i};
            hold on;
            plot(cent(1), cent(2), '-x', 'Color', 'r', 'MarkerSize', 25, 'LineWidth', 8); % red X for taken
            hold off;
            taken = true;
            break;
        end
    end

    if ~taken
        cent = allCentroids{i};
        hold on;
        plot(cent(1), cent(2), '-o', 'Color', 'g', 'MarkerSize', 25, 'LineWidth', 8); % green O for available
        hold off;
    end
end

rect = findall(gcf, 'Type', 'Rectangle'); % detect all rectangles
delete(rect);                             % delete rectangles after analysis
fig8.Name = 'Results';
title('Marked Spaces. Green O = Available. Red X = Taken.');


