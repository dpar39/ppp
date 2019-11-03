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

export function rotatedRectangle(center: Point, width: number, height: number, angle: number): Point[] {
  const w2 = width / 2;
  const h2 = height / 2;
  const cosAngle = Math.cos(angle);
  const sinAngle = Math.sin(angle);
  const corners = [];
  for (const p of [[-w2, -h2], [-w2, h2], [w2, h2], [w2, -h2]]) {
    const x = center.x + p[0] * cosAngle - p[1] * sinAngle;
    const y = center.y + p[0] * sinAngle + p[1] * cosAngle;
    corners.push(new Point(x, y));
  }
  return corners;
}
