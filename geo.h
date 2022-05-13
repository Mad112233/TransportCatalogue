#pragma once

const double RADIUS = 6371000;

namespace geo {
    struct Coordinates {
        double lat;
        double lng;
    };

    double ComputeDistance(Coordinates from, Coordinates to);
}