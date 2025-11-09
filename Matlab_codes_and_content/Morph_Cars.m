function y = Morph_Cars(car,se)



 x = close(car, se);

 y =  open(x , se);
 
 

end

function erodedImage = erosion(binaryImage, structElem)
    % Get dimensions of the binary image and structuring element
    [imgRows, imgCols] = size(binaryImage);
    [seRows, seCols] = size(structElem);

    % Compute padding size for both even and odd kernel sizes
% Compute padding size for even and odd kernel sizes
    padTop = floor((seRows - 1) / 2);
    padBottom = ceil((seRows - 1) / 2);
    padLeft = floor((seCols - 1) / 2);
    padRight = ceil((seCols - 1) / 2);
    %{
 do this for C 

    const int padTop = (SE_ROWS - 1) / 2;
    const int padBottom = SE_ROWS - padTop - 1;
    const int padLeft = (SE_COLS - 1) / 2;
    const int padRight = SE_COLS - padLeft - 1;
    %}


    % Create padded image (manually adding zeros around the edges)
    paddedImage = zeros(imgRows + padTop + padBottom, imgCols + padLeft + padRight);
    for i = 1:imgRows
        for j = 1:imgCols
            paddedImage(i + padTop, j + padLeft) = binaryImage(i, j);
        end
    end

    % Initialize the eroded image
    erodedImage = zeros(imgRows, imgCols);

    % Perform erosion
    for i = 1:imgRows
        for j = 1:imgCols
            match = 1; % Assume the structuring element matches
            for m = 1:seRows
                for n = 1:seCols
                    % Check if structuring element and region match
                    if structElem(m, n) == 1 && paddedImage(i + m - 1, j + n - 1) == 0
                        match = 0; % Mismatch found
                        break;
                    end
                end
                if match == 0
                    break;
                end
            end
            erodedImage(i, j) = match; % Assign result to output image
        end
    end
end

% Dilation function
function dilatedImage = dilation(binaryImage, structElem)
    % Get dimensions of the binary image and structuring element
    [imgRows, imgCols] = size(binaryImage);
    [seRows, seCols] = size(structElem);

    % Compute padding size for even and odd kernel sizes
    padTop = floor((seRows - 1) / 2);
    padBottom = ceil((seRows - 1) / 2);
    padLeft = floor((seCols - 1) / 2);
    padRight = ceil((seCols - 1) / 2);

    % Create padded image (manually adding zeros around the edges)
    paddedImage = zeros(imgRows + padTop + padBottom, imgCols + padLeft + padRight);
    for i = 1:imgRows
        for j = 1:imgCols
            paddedImage(i + padTop, j + padLeft) = binaryImage(i, j);
        end
    end

    % Initialize the dilated image
    dilatedImage = zeros(imgRows, imgCols);

    % Perform dilation
    for i = 1:imgRows
        for j = 1:imgCols
            match = 0; % Assume no match initially
            for m = 1:seRows
                for n = 1:seCols
                    % Check if structuring element overlaps with the region
                    if structElem(m, n) == 1 && paddedImage(i + m - 1, j + n - 1) == 1
                        match = 1; % Match found
                        break;
                    end
                end
                if match == 1
                    break;
                end
            end
            dilatedImage(i, j) = match; % Assign result to output image
        end
    end
end



% closing function

function closing = close(image ,structElem )
%useing dialation followed by a erosion
preim = dilation(image,structElem);
closing = erosion(preim,structElem);
end

function opening = open(image ,structElem )
%useing erosion followed by a dialation
preim = erosion(image,structElem);
opening = dilation(preim,structElem);
end