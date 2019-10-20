import {Point} from './datatypes';

export function middlePoint(pt1: Point, pt2: Point): Point {
    return new Point((pt1.x + pt2.x) / 2, (pt1.y + pt2.y) / 2);
}

export function pointAtDistance(p0: Point, p1: Point, dist: number) {
    if (p1.equals(p0)) {
        throw new Error('Input points cannot be equal');
    }
    const ratio = dist / p1.sub(p0).norm();
    return p0.add(p1.sub(p0).mult(ratio));
}

export function pointsAtDistanceNorm(p1: Point, p2: Point, d: number): Point[] {
    if (p1.equals(p2)) {
        throw new Error('Input points cannot be equal');
    }

    const p0 = p1.add(p2).div(2.0);
    let pa: Point, pb: Point; // Points at distance d from the normal line passing from the center of p1 and p2 (i.e. p0)
    if (p1.x === p2.x) {
        pa = pb = p0;
        pa.x -= d;
        pb.x += d;
    } else if (p1.y === p2.y) {
        pa = pb = p0;
        pa.y -= d;
        pb.y += d;
    } else {
        const m = (p1.x - p2.x) / (p2.y - p1.y); // m' = -1/m
        let dx = d / Math.sqrt(1 + m * m);
        if (m < 0) {
            dx = -dx;
        }
        pa.x = p0.x + dx;
        pb.x = p0.x - dx;
        pa.y = m * (pa.x - p0.x) + p0.y;
        pb.y = m * (pb.x - p0.x) + p0.y;
    }
    return [pa, pb];
}
