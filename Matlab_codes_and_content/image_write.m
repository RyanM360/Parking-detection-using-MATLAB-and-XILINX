%% writeing image to text MATLAB script to use in vitis
% Ryan Mark

% Read the input grayscale image
img = imread('final_parkmask.bmp'); % Change filename if needed
img = uint8(img)*255;%% normalize back to unsigned 8 bit for the VGA in vitis


% sav resized colorimage

% Open a text file for writing
fid = fopen('parking_mask.txt', 'w');

% Get the size of the resized image
[rows, cols] = size(img);

% Iterate over each pixel and write grayscale values with a comma in between
for r = 1:rows
    for c = 1:cols
        
            % Print pixel value with a comma if it's not the last pixel in the row
            fprintf(fid, '%d, ', img(r, c));
       
    end
   % fprintf(fid, '\n'); % New line at the end of each row
end

% Close the file
fclose(fid);

disp('Grayscale image resized, saved as BMP, and pixel values stored in parkmask.txt with commas.');


%% writing extracted car mask to textfile 
% Read the input grayscale image
img = imread('final_carmask.bmp'); % Change filename if needed
img = uint8(img)*255;
% Resize the image to 640x452
%%resized_img = uint8(imresize(img, [452 640])) *255; % recconvert;
%img = imresize(img, [452 640]); % recconvert;

% Save the resized grayscale image as BMP
%%imwrite(resized_img, 'carmask.bmp');

% sav resized colorimage

% Open a text file for writing
fid = fopen('car_mask.txt', 'w');

% Get the size of the resized image
[rows, cols] = size(img);

% Iterate over each pixel and write grayscale values with a comma in between
for r = 1:rows
    for c = 1:cols
        
            % Print pixel value with a comma if it's not the last pixel in the row
            fprintf(fid, '%d, ', img(r, c));
       
    end
   % fprintf(fid, '\n'); % New line at the end of each row
end

% Close the file
fclose(fid);

disp('Grayscale image resized, saved as BMP, and pixel values stored in parkmask.txt with commas.');

%}

%% Colored Image writeing
img = imread('colored_car.bmp'); % Change filename if needed
% Get the size of the resized image
%
[rows, cols, channels] = size(img);
% Iterate over each pixel and write RGB values with a comma in between
fid = fopen('color_car.txt', 'w');
for r = 1:rows
    for c = 1:cols
    hex_value = uint32(img(r, c, 1)) * 65536 + ... % Shift Red to bits 16-23
                    uint32(img(r, c, 2)) * 256 + ...   % Shift Green to bits 8-15
                    uint32(img(r, c, 3));
        fprintf(fid, '0x%06X, ', hex_value);
    end
    %fprintf(fid, '\n'); % New line at the end of each row
end

% Close the file
fclose(fid);

disp('Image resized, saved as BMP, and RGB values stored in RGB_Values.txt with commas.');
%}