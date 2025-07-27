import * as utils from '../core/utils.js'

export function setAnimationProgress (fx: PIXI.AnimatedSprite, progress: number): number {
  let idx = Math.floor(progress * fx.totalFrames)
  idx = Math.min(fx.totalFrames - 1, idx)
  fx.gotoAndStop(idx)
  return idx
}

export function distance (a: PIXI.IPointData, b: PIXI.IPointData) {
  return Math.sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y))
}

export function fit (entity: PIXI.DisplayObject & {width: number, height: number}, maxWidth: number, maxHeight: number): void {
  entity.scale.set(utils.fitAspectRatio(entity.width, entity.height, maxWidth, maxHeight))
}

export interface Point {
  x: number
  y: number
}

export function setSize (sprite: PIXI.Sprite | PIXI.Container, size: number): void {
  sprite.width = size
  sprite.height = size
}

export function bounce (t: number): number {
  return 1 + (Math.sin(t * 10) * 0.5 * Math.cos(t * 3.14 / 2)) * (1 - t) * (1 - t)
}
export function generateText (text: string | number, color: number, size: number, strokeThickness = 4): PIXI.Text {
  const drawnText = new PIXI.Text(typeof text === 'number' ? '' + text : text, {
    fontSize: Math.round(size) + 'px',
    fontFamily: 'Arial',
    fill: color,
    stroke: 0x0,
    strokeThickness,
    lineHeight: Math.round(size),
    align: 'center'
  })
  drawnText.anchor.x = 0.5
  drawnText.anchor.y = 0.5
  return drawnText
}

export function last<T> (arr: T[]): T {
  return arr[arr.length - 1]
}

export function keyOf (x: number, y: number): string {
  return `${x},${y}`
}
export function angleDiff (a: number, b: number): number {
  return Math.abs(utils.lerpAngle(a, b, 0) - utils.lerpAngle(a, b, 1))
}
export function pad (text: string|number, n: number, char: string = '0'): string {
  let out = text.toString()
  while (out.length < n) {
    out = char + out
  }
  return out
}

export function choice<T>(arr: T[]): T {
  if (arr.length === 0) {
    return null as T
  }
  const idx = Math.floor(Math.random() * arr.length)
  return arr[idx]
}

export function randomChoice (rand: number, coeffs: number[]): number {
  const total = coeffs.reduce((a, b) => a + b, 0)
  const b = 1 / total
  const weights = coeffs.map(v => v * b)
  let cur = 0
  for (let i = 0; i < weights.length; ++i) {
    cur += weights[i]
    if (cur >= rand) {
      return i
    }
  }
  return 0
}

export function sum (arr: number[]): number {
  return arr.reduce((a, b) => a + b, 0)
}

export function drawDebugFrameAroundObject (o: any, col = 0xFF00FF, ancX?: number, ancY?: number) {
  const frame = new PIXI.Graphics()
  const x = -o.width * (o.anchor?.x ?? ancX)
  const y = -o.height * (o.anchor?.y ?? ancY)
  frame.beginFill(col, 1)
  frame.drawRect(x, y, o.width, o.height)
  frame.position.copyFrom(o)
  return frame
}