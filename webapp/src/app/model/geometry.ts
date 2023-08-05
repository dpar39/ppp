export class Point {
  x: number;
  y: number;
  constructor(x: number, y: number) {
    this.x = x;
    this.y = y;
  }

  equals(pt: Point): boolean {
    return pt.x === this.x && pt.y === this.y;
  }
  norm(): number {
    return Math.sqrt(this.x * this.x + this.y * this.y);
  }

  add(pt: Point): Point {
    return new Point(this.x + pt.x, this.y + pt.y);
  }

  sub(pt: Point): Point {
    return new Point(this.x - pt.x, this.y - pt.y);
  }

  mult(scalar: number) {
    return new Point(this.x * scalar, this.y * scalar);
  }

  div(scalar: number) {
    return new Point(this.x / scalar, this.y / scalar);
  }

  distTo(pt: Point): number {
    const dx = this.x - pt.x;
    const dy = this.y - pt.y;
    return Math.sqrt(dx * dx + dy * dy);
  }

  angle(pt: Point): number {
    const dx = this.x - pt.x;
    const dy = this.y - pt.y;
    return Math.atan2(dy, dx);
  }
}

export class Rect {
  constructor(public cx: number, public cy: number, public width: number, public height: number) {}
  topLeft(): Point {
    return new Point(this.cx - this.width / 2, this.cy - this.height / 2);
  }
  bottomRight(): Point {
    return new Point(this.cx + this.width / 2, this.cy + this.height / 2);
  }
}

export class RotatedRect {
  constructor(public center: Point, public length1: number, public length2: number, public angle: number) {}

  corners(): Point[] {
    const w2 = this.length1 / 2;
    const h2 = this.length2 / 2;
    const cosAngle = Math.cos(this.angle);
    const sinAngle = Math.sin(this.angle);
    const corners = [];
    for (const p of [
      [-w2, -h2],
      [-w2, h2],
      [w2, h2],
      [w2, -h2],
    ]) {
      const x = this.center.x + p[0] * cosAngle - p[1] * sinAngle;
      const y = this.center.y + p[0] * sinAngle + p[1] * cosAngle;
      corners.push(new Point(x, y));
    }
    return corners;
  }

  boundingBox() {
    const corners = this.corners();
    const c0 = corners[0];
    let x1 = c0.x;
    let x2 = c0.x;
    let y1 = c0.y;
    let y2 = c0.y;
    for (const c of corners) {
      if (c.x < x1) x1 = c.x;
      else if (c.x > x2) x2 = c.x;
      else if (c.y < y1) y1 = c.y;
      else if (c.y > y2) y2 = c.y;
    }
    return new Rect((x1 + x2) / 2, (y1 + y2) / 2, x2 - x1, y2 - y1);
  }
}

export function middlePoint(pt1: Point, pt2: Point): Point {
  return new Point((pt1.x + pt2.x) / 2, (pt1.y + pt2.y) / 2);
}

export function pointAtDistance(p0: Point, p1: Point, dist: number): Point {
  if (p1.equals(p0)) {
    throw new Error('Input points cannot be equal');
  }
  const ratio = dist / p1.sub(p0).norm();
  return p0.add(p1.sub(p0).mult(ratio));
}

export function pointsAtDistanceNorm(p1: Point, p2: Point, d: number, p0: Point = null): Point[] {
  if (p1.equals(p2)) {
    throw new Error('Input points cannot be equal');
  }

  if (p0 == null) {
    p0 = p1.add(p2).div(2.0);
  }
  const pa = new Point(0, 0);
  const pb = new Point(0, 0); // Points at distance d from the normal line passing from the center of p1 and p2 (i.e. p0)
  if (p1.x === p2.x) {
    pa.y = pb.y = p0.y;
    pa.x = p0.x - d;
    pb.x = p0.x + d;
  } else if (p1.y === p2.y) {
    pa.y = pb.y = p0.y;
    pa.x = pb.x = p0.x;
    pa.y = p0.y - d;
    pb.y = p0.y + d;
  } else {
    const ddx = p2.x - p1.x;
    const ddy = p2.y - p1.y;
    const m = -ddx / ddy; // m' = -1/m
    let dx = d / Math.sqrt(1 + m * m);
    if (ddy < 0) {
      dx = -dx;
    }
    pa.x = p0.x + dx;
    pb.x = p0.x - dx;
    pa.y = m * (pa.x - p0.x) + p0.y;
    pb.y = m * (pb.x - p0.x) + p0.y;
  }
  return [pa, pb];
}

export function translateSegmentParallel(p1: Point, p2: Point, d: number) {
  const p1s = pointsAtDistanceNorm(p1, p2, d, p1);
  const p2s = pointsAtDistanceNorm(p1, p2, d, p2);
  return [p1s[0], p2s[0]];
}

export function toMillimeters(v: number, units: string) {
  switch (units) {
    case 'mm':
      return v;
    case 'inch':
      return v * 25.4;
    case 'cm':
      return v * 10.0;
    default:
      throw new Error('Unknown input units when creating the photo standard definition');
  }
}

export function getRotatedRectBB(x, y, width: number, height: number, angle) {}

export function fromPixel(v: number, units: string, dpi: number): number {
  if (units === 'pixel') {
    return v;
  }
  if (units === 'inch') {
    return v / dpi;
  }
  if (units === 'mm') {
    return (v / dpi) * 25.4;
  }
  if (units === 'cm') {
    return (v / dpi) * 2.54;
  }
  throw new Error('Unknown input units when converting to pixels');
}

export function toPixels(v: number, units: string, dpi: number) {
  if (units === 'pixel') {
    return v;
  }
  if (units === 'inch') {
    return v * dpi;
  }
  if (units === 'mm') {
    return (v * dpi) / 25.4;
  }
  if (units === 'cm') {
    return (v * dpi) / 2.54;
  }
  throw new Error('Unknown input units when converting to pixels');
}

export function convert(v: number, from: string, to: string, dpi: number) {
  if (from === to) {
    return v; // Nothing to convert, great!
  }
  const pixEquivalent = toPixels(v, from, dpi);
  return fromPixel(pixEquivalent, to, dpi);
}

/**
 * Returns a matrix that transforms a triangle t1 into another triangle t2, or throws an exception if it is impossible.
 * @param t1 {Point[]} Array of points containing the three points for the first triangle
 * @param t2 {Point[]} Array of points containing the three points for the second triangle
 * @returns {Matrix} Matrix which transforms t1 to t2
 * @throws Exception if the matrix becomes not invertible
 */
export function getAffineTransform(fr: Point[], to: Point[]) {
  const x0 = to[0].x;
  const y0 = to[0].y;
  const x1 = to[1].x;
  const y1 = to[1].y;
  const x2 = to[2].x;
  const y2 = to[2].y;
  const u0 = fr[0].x;
  const v0 = fr[0].y;
  const u1 = fr[1].x;
  const v1 = fr[1].y;
  const u2 = fr[2].x;
  const v2 = fr[2].y;

  // Compute matrix transform
  const delta = u0 * v1 + v0 * u2 + u1 * v2 - v1 * u2 - v0 * u1 - u0 * v2;
  const deltaA = x0 * v1 + v0 * x2 + x1 * v2 - v1 * x2 - v0 * x1 - x0 * v2;
  const deltaB = u0 * x1 + x0 * u2 + u1 * x2 - x1 * u2 - x0 * u1 - u0 * x2;
  const deltaC = u0 * v1 * x2 + v0 * x1 * u2 + x0 * u1 * v2 - x0 * v1 * u2 - v0 * u1 * x2 - u0 * x1 * v2;
  const deltaD = y0 * v1 + v0 * y2 + y1 * v2 - v1 * y2 - v0 * y1 - y0 * v2;
  const deltaE = u0 * y1 + y0 * u2 + u1 * y2 - y1 * u2 - y0 * u1 - u0 * y2;
  const deltaF = u0 * v1 * y2 + v0 * y1 * u2 + y0 * u1 * v2 - y0 * v1 * u2 - v0 * u1 * y2 - u0 * y1 * v2;

  // Draw the transformed image
  return [deltaA / delta, deltaD / delta, deltaB / delta, deltaE / delta, deltaC / delta, deltaF / delta];
}
