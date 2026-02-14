//===----------------------------------------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2016 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

import CDispatch
import _SwiftDispatchOverlayShims

// This file contains declarations that are provided by the
// importer via Dispatch.apinote when the platform has Objective-C support

public func dispatchMain() -> Never {
	CDispatch.dispatch_main()
}

public class DispatchObject {

	internal func wrapped() -> dispatch_object_t {
		fatalError("should be overriden in subclass")
	}

	public func setTarget(queue:DispatchQueue) {
		dispatch_set_target_queue(wrapped(), queue.__wrapped)
	}

	public func activate() {
		dispatch_activate(wrapped())
	}

	public func suspend() {
		dispatch_suspend(wrapped())
	}

	public func resume() {
		dispatch_resume(wrapped())
	}
}


public class DispatchGroup : DispatchObject {
	internal let __wrapped:dispatch_group_t;

	final internal override func wrapped() -> dispatch_object_t {
		return unsafeBitCast(__wrapped, to: dispatch_object_t.self)
	}

	public override init() {
		__wrapped = dispatch_group_create()
	}

	deinit {
		_swift_dispatch_release(wrapped())
	}

	public func enter() {
		dispatch_group_enter(__wrapped)
	}

	public func leave() {
		dispatch_group_leave(__wrapped)
	}
}

public class DispatchSemaphore : DispatchObject {
	internal let __wrapped: dispatch_semaphore_t;

	final internal override func wrapped() -> dispatch_object_t {
		return unsafeBitCast(__wrapped, to: dispatch_object_t.self)
	}

	public init(value: Int) {
		__wrapped = dispatch_semaphore_create(Int(value))
	}

	deinit {
		_swift_dispatch_release(wrapped())
	}
}

public class DispatchIO : DispatchObject {
	internal let __wrapped:dispatch_io_t

	final internal override func wrapped() -> dispatch_object_t {
		return unsafeBitCast(__wrapped, to: dispatch_object_t.self)
	}

	internal init(__type: UInt, fd: Int32, queue: DispatchQueue,
				  handler: @escaping (_ error: Int32) -> Void) {
		__wrapped = dispatch_io_create(dispatch_io_type_t(__type), dispatch_fd_t(fd), queue.__wrapped, handler)
	}

	internal init(__type: UInt, path: UnsafePointer<Int8>, oflag: Int32,
				  mode: mode_t, queue: DispatchQueue, handler: @escaping (_ error: Int32) -> Void) {
		__wrapped = dispatch_io_create_with_path(dispatch_io_type_t(__type), path, oflag, mode, queue.__wrapped, handler)
	}

	internal init(__type: UInt, io: DispatchIO,
				  queue: DispatchQueue, handler: @escaping (_ error: Int32) -> Void) {
		__wrapped = dispatch_io_create_with_io(dispatch_io_type_t(__type), io.__wrapped, queue.__wrapped, handler)
	}

	deinit {
		_swift_dispatch_release(wrapped())
	}

	public func barrier(execute: @escaping () -> ()) {
		dispatch_io_barrier(self.__wrapped, execute)
	}

	public var fileDescriptor: Int32 {
		return Int32(dispatch_io_get_descriptor(__wrapped))
	}

	public func setLimit(highWater: Int) {
		dispatch_io_set_high_water(__wrapped, highWater)
	}

	public func setLimit(lowWater: Int) {
		dispatch_io_set_low_water(__wrapped, lowWater)
	}
}

public class DispatchQueue : DispatchObject {
	internal let __wrapped:dispatch_queue_t;

	final internal override func wrapped() -> dispatch_object_t {
		return unsafeBitCast(__wrapped, to: dispatch_object_t.self)
	}

	internal init(__label: String, attr: dispatch_queue_attr_t?) {
		__wrapped = dispatch_queue_create(__label, attr)
	}

	internal init(__label: String, attr:  dispatch_queue_attr_t?, queue: DispatchQueue?) {
		__wrapped = dispatch_queue_create_with_target(__label, attr, queue?.__wrapped)
	}

	internal init(queue:dispatch_queue_t) {
		__wrapped = queue
	}

	deinit {
		_swift_dispatch_release(wrapped())
	}

	public func sync(execute workItem: ()->()) {
		dispatch_sync(self.__wrapped, workItem)
	}
}

public class DispatchSource : DispatchObject,
	DispatchSourceProtocol,	DispatchSourceRead,
	DispatchSourceSignal, DispatchSourceTimer,
	DispatchSourceUserDataAdd, DispatchSourceUserDataOr,
	DispatchSourceUserDataReplace, DispatchSourceWrite {
	internal let __wrapped:dispatch_source_t

	final internal override func wrapped() -> dispatch_object_t {
		return unsafeBitCast(__wrapped, to: dispatch_object_t.self)
	}

	internal init(source:dispatch_source_t) {
		__wrapped = source
	}

	deinit {
		_swift_dispatch_release(wrapped())
	}
}

#if HAVE_MACH
extension DispatchSource : DispatchSourceMachSend,
	DispatchSourceMachReceive, DispatchSourceMemoryPressure {
}
#endif

#if !os(Linux) && !os(Android)
extension DispatchSource : DispatchSourceProcess,
	DispatchSourceFileSystemObject {
}
#endif

internal class __DispatchData : DispatchObject {
	internal let __wrapped:dispatch_data_t

	final internal override func wrapped() -> dispatch_object_t {
		return unsafeBitCast(__wrapped, to: dispatch_object_t.self)
	}

	internal init(data:dispatch_data_t, owned:Bool) {
		__wrapped = data
		if !owned {
			_swift_dispatch_retain(unsafeBitCast(data, to: dispatch_object_t.self))
		}
	}

	deinit {
		_swift_dispatch_release(wrapped())
	}
}

public typealias DispatchSourceHandler = @convention(block) () -> Void

public protocol DispatchSourceProtocol {
	func setEventHandler(qos: DispatchQoS, flags: DispatchWorkItemFlags, handler: DispatchSourceHandler?)

	func setEventHandler(handler: DispatchWorkItem)

	func setCancelHandler(qos: DispatchQoS, flags: DispatchWorkItemFlags, handler: DispatchSourceHandler?)

	func setCancelHandler(handler: DispatchWorkItem)

	func setRegistrationHandler(qos: DispatchQoS, flags: DispatchWorkItemFlags, handler: DispatchSourceHandler?)

	func setRegistrationHandler(handler: DispatchWorkItem)

	func cancel()

	func resume()

	func suspend()

	var handle: UInt { get }

	var mask: UInt { get }

	var data: UInt { get }

	var isCancelled: Bool { get }
}

public protocol DispatchSourceUserDataAdd : DispatchSourceProtocol {
	func add(data: UInt)
}

public protocol DispatchSourceUserDataOr : DispatchSourceProtocol {
	func or(data: UInt)
}

public protocol DispatchSourceUserDataReplace : DispatchSourceProtocol {
	func replace(data: UInt)
}

#if HAVE_MACH
public protocol DispatchSourceMachSend : DispatchSourceProtocol {
	public var handle: mach_port_t { get }

	public var data: DispatchSource.MachSendEvent { get }

	public var mask: DispatchSource.MachSendEvent { get }
}
#endif

#if HAVE_MACH
public protocol DispatchSourceMachReceive : DispatchSourceProtocol {
	var handle: mach_port_t { get }
}
#endif

#if HAVE_MACH
public protocol DispatchSourceMemoryPressure : DispatchSourceProtocol {
	public var data: DispatchSource.MemoryPressureEvent { get }

	public var mask: DispatchSource.MemoryPressureEvent { get }
}
#endif

#if !os(Linux) && !os(Android)
public protocol DispatchSourceProcess : DispatchSourceProtocol {
	var handle: pid_t { get }

	var data: DispatchSource.ProcessEvent { get }

	var mask: DispatchSource.ProcessEvent { get }
}
#endif

public protocol DispatchSourceRead : DispatchSourceProtocol {
}

public protocol DispatchSourceSignal : DispatchSourceProtocol {
}

public protocol DispatchSourceTimer : DispatchSourceProtocol {
	func scheduleOneshot(deadline: DispatchTime, leeway: DispatchTimeInterval)

	func scheduleOneshot(wallDeadline: DispatchWallTime, leeway: DispatchTimeInterval)

	func scheduleRepeating(deadline: DispatchTime, interval: DispatchTimeInterval, leeway: DispatchTimeInterval)

	func scheduleRepeating(deadline: DispatchTime, interval: Double, leeway: DispatchTimeInterval)

	func scheduleRepeating(wallDeadline: DispatchWallTime, interval: DispatchTimeInterval, leeway: DispatchTimeInterval)

	func scheduleRepeating(wallDeadline: DispatchWallTime, interval: Double, leeway: DispatchTimeInterval)
}

#if !os(Linux) && !os(Android)
public protocol DispatchSourceFileSystemObject : DispatchSourceProtocol {
	var handle: Int32 { get }

	var data: DispatchSource.FileSystemEvent { get }

	var mask: DispatchSource.FileSystemEvent { get }
}
#endif

public protocol DispatchSourceWrite : DispatchSourceProtocol {
}


internal enum _OSQoSClass : UInt32  {
	case QOS_CLASS_USER_INTERACTIVE = 0x21
	case QOS_CLASS_USER_INITIATED = 0x19
	case QOS_CLASS_DEFAULT = 0x15
	case QOS_CLASS_UTILITY = 0x11
	case QOS_CLASS_BACKGROUND = 0x09
	case QOS_CLASS_UNSPECIFIED = 0x00

	internal init?(qosClass:dispatch_qos_class_t) {
		switch qosClass {
		case 0x21: self = .QOS_CLASS_USER_INTERACTIVE
		case 0x19: self = .QOS_CLASS_USER_INITIATED
		case 0x15: self = .QOS_CLASS_DEFAULT
		case 0x11: self = .QOS_CLASS_UTILITY
		case 0x09: self = .QOS_CLASS_BACKGROUND
		case 0x00: self = .QOS_CLASS_UNSPECIFIED
		default: return nil
		}
	}
}
