/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Original file: /home/wtao/workspace/ScreenCoordsService/src/me/wtao/service/IScreenCoordsService.aidl
 */
package me.wtao.service;
public interface IScreenCoordsService extends android.os.IInterface
{
/** Local-side IPC implementation stub class. */
public static abstract class Stub extends android.os.Binder implements me.wtao.service.IScreenCoordsService
{
private static final java.lang.String DESCRIPTOR = "me.wtao.service.IScreenCoordsService";
/** Construct the stub at attach it to the interface. */
public Stub()
{
this.attachInterface(this, DESCRIPTOR);
}
/**
 * Cast an IBinder object into an me.wtao.service.IScreenCoordsService interface,
 * generating a proxy if needed.
 */
public static me.wtao.service.IScreenCoordsService asInterface(android.os.IBinder obj)
{
if ((obj==null)) {
return null;
}
android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
if (((iin!=null)&&(iin instanceof me.wtao.service.IScreenCoordsService))) {
return ((me.wtao.service.IScreenCoordsService)iin);
}
return new me.wtao.service.IScreenCoordsService.Stub.Proxy(obj);
}
@Override public android.os.IBinder asBinder()
{
return this;
}
@Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
{
switch (code)
{
case INTERFACE_TRANSACTION:
{
reply.writeString(DESCRIPTOR);
return true;
}
case TRANSACTION_obtainMotionEvent:
{
data.enforceInterface(DESCRIPTOR);
android.view.MotionEvent _result = this.obtainMotionEvent();
reply.writeNoException();
if ((_result!=null)) {
reply.writeInt(1);
_result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
}
else {
reply.writeInt(0);
}
return true;
}
}
return super.onTransact(code, data, reply, flags);
}
private static class Proxy implements me.wtao.service.IScreenCoordsService
{
private android.os.IBinder mRemote;
Proxy(android.os.IBinder remote)
{
mRemote = remote;
}
@Override public android.os.IBinder asBinder()
{
return mRemote;
}
public java.lang.String getInterfaceDescriptor()
{
return DESCRIPTOR;
}
@Override public android.view.MotionEvent obtainMotionEvent() throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
android.view.MotionEvent _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
mRemote.transact(Stub.TRANSACTION_obtainMotionEvent, _data, _reply, 0);
_reply.readException();
if ((0!=_reply.readInt())) {
_result = android.view.MotionEvent.CREATOR.createFromParcel(_reply);
}
else {
_result = null;
}
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
}
static final int TRANSACTION_obtainMotionEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
}
public android.view.MotionEvent obtainMotionEvent() throws android.os.RemoteException;
}
