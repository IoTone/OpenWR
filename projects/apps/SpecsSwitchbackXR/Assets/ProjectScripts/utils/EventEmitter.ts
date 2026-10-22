// eventEmitter.ts VIBE ported 
// from: https://github.com/nodejs/node/blob/v20.19.2/lib/events.js
// eventEmitter.ts
// IoTone, Inc. 2025
// Type for event listeners
type Listener<T extends any[] = any[]> = (...args: T) => void;

// Event map interface for type safety
interface EventMap {
  [event: string]: any[];
}

// Default error handler for 'error' event
function defaultErrorHandler(err: any): void {
  print(`Unhandled error event: ${err}`);
  throw err;
}

export class EventEmitter<TEventMap extends EventMap = Record<string, any[]>> {
  private _events: Map<keyof TEventMap, Listener<TEventMap[keyof TEventMap]>[]> = new Map();
  private _maxListeners: number = 10;
  private static defaultMaxListeners: number = 10;

  constructor() {
    this._events = new Map();
  }

  // Add a listener for an event
  on<K extends keyof TEventMap>(event: K, listener: Listener<TEventMap[K]>): this {
    this._addListener(event, listener, false);
    return this;
  }

  // Add a one-time listener
  once<K extends keyof TEventMap>(event: K, listener: Listener<TEventMap[K]>): this {
    const wrapped: Listener<TEventMap[K]> = (...args: TEventMap[K]) => {
      this.off(event, wrapped);
      listener(...args);
    };
    // Preserve raw listener for rawListeners
    (wrapped as any).listener = listener;
    this._addListener(event, wrapped, false);
    return this;
  }

  // Add a listener at the beginning of the array
  prependListener<K extends keyof TEventMap>(event: K, listener: Listener<TEventMap[K]>): this {
    this._addListener(event, listener, true);
    return this;
  }

  // Add a one-time listener at the beginning
  prependOnceListener<K extends keyof TEventMap>(event: K, listener: Listener<TEventMap[K]>): this {
    const wrapped: Listener<TEventMap[K]> = (...args: TEventMap[K]) => {
      this.off(event, wrapped);
      listener(...args);
    };
    (wrapped as any).listener = listener;
    this._addListener(event, wrapped, true);
    return this;
  }

  // Remove a specific listener or all listeners for an event
  off<K extends keyof TEventMap>(event: K, listener?: Listener<TEventMap[K]>): this {
    if (!listener) {
      this._events.delete(event);
      return this;
    }

    const listeners = this._events.get(event);
    if (listeners) {
      const index = listeners.indexOf(listener);
      if (index !== -1) {
        listeners.splice(index, 1);
        if (listeners.length === 0) {
          this._events.delete(event);
        }
      }
    }
    return this;
  }

  // Emit an event with arguments
  emit<K extends keyof TEventMap>(event: K, ...args: TEventMap[K]): boolean {
    const listeners = this._events.get(event);
    if (!listeners || listeners.length === 0) {
      if (event === 'error') {
        const err = args[0] || new Error('Unhandled error');
        defaultErrorHandler(err);
      }
      return false;
    }

    // Copy listeners to handle modifications during emission
    const listenersCopy = [...listeners];
    for (const listener of listenersCopy) {
      try {
        listener(...args);
      } catch (err) {
        print(`Error in listener for "${String(event)}": ${err}`);
      }
    }
    return true;
  }

  // Get all listeners for an event
  listeners<K extends keyof TEventMap>(event: K): Listener<TEventMap[K]>[] {
    const listeners = this._events.get(event);
    return listeners ? [...listeners] : [];
  }

  // Get raw listeners (including wrapped once listeners)
  rawListeners<K extends keyof TEventMap>(event: K): Listener<TEventMap[K]>[] {
    const listeners = this._events.get(event);
    if (!listeners) return [];
    return listeners.map(listener => {
      return (listener as any).listener || listener;
    });
  }

  // Get count of listeners for an event
  listenerCount<K extends keyof TEventMap>(event: K): number {
    const listeners = this._events.get(event);
    return listeners ? listeners.length : 0;
  }

  // Get all event names
  eventNames(): (keyof TEventMap)[] {
    return Array.from(this._events.keys());
  }

  // Remove all listeners for all events or a specific event
  removeAllListeners(event?: keyof TEventMap): this {
    if (event === undefined) {
      this._events.clear();
    } else {
      this._events.delete(event);
    }
    return this;
  }

  // Set maximum number of listeners per event
  setMaxListeners(n: number): this {
    if (n < 0 || !Number.isInteger(n)) {
      print('Warning: maxListeners must be a non-negative integer');
      return this;
    }
    this._maxListeners = n;
    return this;
  }

  // Get maximum number of listeners
  getMaxListeners(): number {
    return this._maxListeners;
  }

  // Alias for on
  addListener<K extends keyof TEventMap>(event: K, listener: Listener<TEventMap[K]>): this {
    return this.on(event, listener);
  }

  // Alias for off
  removeListener<K extends keyof TEventMap>(event: K, listener: Listener<TEventMap[K]>): this {
    return this.off(event, listener);
  }

  // Static method to initialize once wrapping
  static once<T extends EventMap, K extends keyof T>(
    emitter: EventEmitter<T>,
    event: K
  ): Promise<T[K]> {
    return new Promise((resolve) => {
      emitter.once(event, (...args: T[K]) => resolve(args));
    });
  }

  // Static method to set default max listeners
  static setDefaultMaxListeners(n: number): void {
    if (n < 0 || !Number.isInteger(n)) {
      print('Warning: defaultMaxListeners must be a non-negative integer');
      return;
    }
    EventEmitter.defaultMaxListeners = n;
  }

  private _addListener<K extends keyof TEventMap>(
    event: K,
    listener: Listener<TEventMap[K]>,
    prepend: boolean
  ): void {
    let listeners = this._events.get(event);
    if (!listeners) {
      listeners = [];
      this._events.set(event, listeners);
    }

    if (listeners.length >= this._maxListeners && this._maxListeners > 0) {
      print(
        `Warning: possible EventEmitter memory leak detected. ` +
        `${listeners.length + 1} "${String(event)}" listeners added. ` +
        `Use setMaxListeners() to increase limit.`
      );
    }

    if (prepend) {
      listeners.unshift(listener);
    } else {
      listeners.push(listener);
    }
  }
}