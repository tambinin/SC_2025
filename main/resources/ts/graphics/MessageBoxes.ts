import { flagForDestructionOnReinit } from '../core/rendering.js'
import { ViewModule } from './ViewModule.js'
import { fit } from './utils.js'
import * as utils from '../core/utils.js'
import { WIDTH } from '../core/constants.js'

export function renderMessageContainer (this: ViewModule, messageContainer: MessageContainer, i, step: number) {
  const playerInfo = this.globalData.players
  const options = this.api.options
  const stepFactor = Math.pow(0.99, step)
  const targetMessageAlpha = (options.showMyMessages && playerInfo[i].isMe) || (options.showOthersMessages && !playerInfo[i].isMe) ? 1
    : 0
  messageContainer.alpha = messageContainer.alpha * stepFactor + targetMessageAlpha * (1 - stepFactor)
}

export const messageBox = {
  width: 150,
  offset: {
    x: 36,
    y: -32
  }
}

export type MessageContainer = PIXI.Container & {
  messageText: PIXI.Text
  updateText: (text: string, x: number, y: number) => void
}

export function initMessages (layer) {
  const self: ViewModule = this
  self.messages = []
  for (let i = 0; i < self.globalData.playerCount; ++i) {
    self.messages[i] = []
    const messageGroup = new PIXI.Container()

    for (let k = 0; k < self.globalData.agentsPerPlayer; ++k) {
      const messageContainer = new PIXI.Container() as MessageContainer

      const baseScale = 0.5699481865284974
      const bubble = new PIXI.Container()
      const bubbleLeft = PIXI.Sprite.from('dial_side.png')
      const bubbleRight = PIXI.Sprite.from('dial_side.png')
      const bubbleMid = PIXI.Sprite.from('dial_mid.png')
      const bubbleArrow = PIXI.Sprite.from('dial_arrow.png')

      bubbleLeft.anchor.y = 0.5
      bubbleRight.anchor.y = 0.5
      bubbleMid.anchor.y = 0.5

      bubbleLeft.scale.set(-baseScale, baseScale)
      bubbleRight.scale.set(baseScale, baseScale)
      bubbleMid.scale.set(baseScale, baseScale)
      bubbleArrow.scale.set(baseScale, baseScale)
      bubble.y = -18

      bubbleArrow.y = bubbleMid.height / 2 - 5
      bubbleArrow.x = -25

      bubble.addChild(bubbleLeft)
      bubble.addChild(bubbleRight)
      bubble.addChild(bubbleMid)
      bubble.addChild(bubbleArrow)

      const textStyle: Partial<PIXI.TextStyle> = {
        fontFamily: 'Arial',
        fontWeight: '700',
        fontSize: 23,
        fill: self.globalData.players[i].color,
        align: 'center',
        wordWrap: true,
        wordWrapWidth: 90
      }
      const messageText = new PIXI.Text('', textStyle)

      messageText.anchor.x = 0.5
      messageText.anchor.y = 0.5

      // messageText.y = -20


      flagForDestructionOnReinit(messageText)

      messageContainer.messageText = messageText

      messageContainer.updateText = (text, x, y) => {
        bubbleArrow.visible = false
        bubble.visible = (!!text)
        messageText.text = text
        const maxHeight = 150
        if (messageText.height > maxHeight) {
          while (messageText.text.length > 3 && messageText.height > maxHeight) {
            messageText.text = messageText.text.slice(0, -4) + '...'
          }
        }

        const w = messageText.width
        bubbleLeft.x = -w / 2
        bubbleRight.x = w / 2
        bubbleMid.x = -w / 2
        bubbleMid.width = w

        messageContainer.position.set(x, y)
        // the commented code was used to slightly offsets the y pos according to a factor of its size. 
        messageContainer.y += messageBox.offset.y //* messageText.height / 16
        messageContainer.x += messageBox.offset.x + messageContainer.width / 2


        bubble.height = messageText.height * 1.2

        bubbleArrow.x = -messageContainer.width / 2
        bubbleArrow.visible = true

        //Prevents the message from going out of the screen
        if (messageContainer.x > WIDTH - messageContainer.width/2) {
          const diff = messageContainer.x - (WIDTH - messageContainer.width / 2)
          messageContainer.x -= diff
          bubbleArrow.x += diff
        }

        if (messageText.height > 50) {
          bubble.y = -18
          messageText.y = -20
        } else {
          bubble.y = 0
          messageText.y = 0
        }
      }

      messageContainer.addChild(bubble)
      messageContainer.addChild(messageText)

      self.messages[i].push(messageContainer)
      messageGroup.addChild(messageContainer)
    }
    layer.addChild(messageGroup)
  }
}
