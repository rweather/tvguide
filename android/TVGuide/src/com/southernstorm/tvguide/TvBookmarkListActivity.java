package com.southernstorm.tvguide;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Toast;

public class TvBookmarkListActivity extends Activity {

    private ListView bookmarkListView;
    private TvBookmarkListAdapter bookmarkListAdapter;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.bookmark_list);
        
        bookmarkListView = (ListView)findViewById(R.id.bookmarkList);
        bookmarkListAdapter = new TvBookmarkListAdapter(this);
        bookmarkListView.setAdapter(bookmarkListAdapter);
        registerForContextMenu(bookmarkListView);
        
        bookmarkListView.setOnItemClickListener(new OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View v, int position, long id) {
                if (bookmarkListAdapter.isNewBookmarkItem(position))
                    newBookmark();
                else
                    editBookmark(bookmarkListAdapter.getBookmark(position));
            }
        });
    }

    @Override
    protected void onStart() {
        super.onStart();
        
        TvBookmarkManager.getInstance().addContext(this);
        
        bookmarkListAdapter.attach();
    }

    @Override
    protected void onStop() {
        TvBookmarkManager.getInstance().removeContext(this);
        bookmarkListAdapter.detach();
        super.onStop();
    }

    private static final int ITEM_NEW_BOOKMARK = 1;
    private static final int ITEM_EDIT_BOOKMARK = 2;
    private static final int ITEM_DELETE_BOOKMARK = 3;

    private static final int DIALOG_NEW_BOOKMARK = 10;
    private static final int DIALOG_EDIT_BOOKMARK = 11;
    private static final int DIALOG_DELETE_BOOKMARK = 12;
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(ITEM_NEW_BOOKMARK, 0, 0, "New Bookmark");
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getGroupId() == ITEM_NEW_BOOKMARK) {
            newBookmark();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
    
    private void newBookmark() {
        showBookmarkDialog(DIALOG_NEW_BOOKMARK, null);
    }
    
    private void editBookmark(TvBookmark bookmark) {
        showBookmarkDialog(DIALOG_EDIT_BOOKMARK, bookmark);
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
        AdapterView.AdapterContextMenuInfo info = (AdapterView.AdapterContextMenuInfo)menuInfo;
        TvBookmark bookmark = bookmarkListAdapter.getBookmark(info.position);
        if (bookmark == null)
            return;
        menu.setHeaderTitle("Bookmark Menu");
        menu.add(ITEM_EDIT_BOOKMARK, 0, 0, "Edit Bookmark");
        menu.add(ITEM_DELETE_BOOKMARK, 0, 0, "Delete Bookmark");
    }
    
    @Override
    public boolean onContextItemSelected(MenuItem item) {
        AdapterContextMenuInfo info = (AdapterContextMenuInfo)item.getMenuInfo();
        TvBookmark bookmark = bookmarkListAdapter.getBookmark(info.position);
        if (bookmark == null)
            return false;
        switch (item.getGroupId()) {
        case ITEM_EDIT_BOOKMARK:
            showBookmarkDialog(DIALOG_EDIT_BOOKMARK, bookmark);
            return true;
        case ITEM_DELETE_BOOKMARK:
            showBookmarkDialog(DIALOG_DELETE_BOOKMARK, bookmark);
            return true;
        }
        return false;
    }
    
    private void showBookmarkDialog(int id, TvBookmark bookmark) {
        Bundle bundle = new Bundle();
        if (bookmark != null)
            bundle.putLong("bookmark_id", bookmark.getInternalId());
        removeDialog(id); // Don't reuse previous dialog as the bundle will be different.
        showDialog(id, bundle);
    }

    @Override
    public Dialog onCreateDialog(int id, Bundle bundle) {
        long bookmarkId = bundle.getLong("bookmark_id", -1L);
        final TvBookmark bookmark = TvBookmarkManager.getInstance().findBookmarkById(bookmarkId);
        EditBookmarkDialog dialog;
        switch (id) {
        case DIALOG_NEW_BOOKMARK:
            dialog = new EditBookmarkDialog(this);
            final TvBookmark newBookmark = new TvBookmark();
            dialog.copyFromBookmark(newBookmark);
            dialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
                public void onDismiss(DialogInterface dialog) {
                    if (((EditBookmarkDialog)dialog).copyToBookmark(newBookmark)) {
                        TvBookmarkManager.getInstance().addBookmark(newBookmark);
                        toast("Bookmark added");
                    }
                }
            });
            dialog.setTitle("New Bookmark");
            return dialog;
        case DIALOG_EDIT_BOOKMARK:
            dialog = new EditBookmarkDialog(this);
            dialog.copyFromBookmark(bookmark);
            dialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
                public void onDismiss(DialogInterface dialog) {
                    if (((EditBookmarkDialog)dialog).copyToBookmark(bookmark)) {
                        TvBookmarkManager.getInstance().updateBookmark(bookmark);
                        toast("Bookmark saved");
                    }
                }
            });
            return dialog;
        case DIALOG_DELETE_BOOKMARK:
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setMessage("Are you sure you want to delete this bookmark?")
                   .setCancelable(false)
                   .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                       public void onClick(DialogInterface dialog, int id) {
                           TvBookmarkManager.getInstance().removeBookmark(bookmark);
                           toast("Bookmark deleted");
                           dialog.dismiss();
                       }
                   })
                   .setNegativeButton("No", new DialogInterface.OnClickListener() {
                       public void onClick(DialogInterface dialog, int id) {
                            dialog.cancel();
                       }
                   });
            return builder.create();
        default: break;
        }
        return null;
    }
    
    private void toast(String message) {
        Toast toast = Toast.makeText(this, message, Toast.LENGTH_SHORT);
        toast.show();
    }
}
