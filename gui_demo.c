#include <gtk/gtk.h>

typedef struct {
    GtkWidget *entry;
    GtkWidget *label;
    GtkWidget *check;
    GtkWidget *scale;
    GtkWidget *progress;
} AppWidgets;

// 버튼 클릭 → 라벨에 Entry 텍스트 출력
static void on_button_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *w = (AppWidgets*)user_data;
    const char *text = gtk_entry_get_text(GTK_ENTRY(w->entry));

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w->check))) {
        gtk_label_set_text(GTK_LABEL(w->label), "체크박스: ON");
    }

    gtk_label_set_text(GTK_LABEL(w->label), text);
}

// 슬라이더 움직이면 progress bar 업데이트
static void on_scale_changed(GtkRange *range, gpointer user_data) {
    AppWidgets *w = (AppWidgets*)user_data;
    double val = gtk_range_get_value(range);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(w->progress), val / 100.0);
}

// 주기적으로 progress bar 움직이기 (timer)
gboolean on_timer_tick(gpointer user_data) {
    AppWidgets *w = (AppWidgets*)user_data;
    double frac = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(w->progress));

    frac += 0.01;
    if (frac > 1.0) frac = 0;

    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(w->progress), frac);
    return TRUE;  // TRUE = 타이머 계속 반복
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *entry, *label, *button, *check, *scale, *progress;

    AppWidgets w;

    gtk_init(&argc, &argv);

    // 창 생성
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GUI 위젯 데모");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Grid 레이아웃
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 15);

    // 위젯 생성
    label = gtk_label_new("여기에 결과가 표시됩니다");
    entry = gtk_entry_new();
    button = gtk_button_new_with_label("텍스트 적용");
    check = gtk_check_button_new_with_label("체크박스");
    scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    progress = gtk_progress_bar_new();

    // 구조체에 저장
    w.entry = entry;
    w.label = label;
    w.check = check;
    w.scale = scale;
    w.progress = progress;

    // Grid 배치
    gtk_grid_attach(GTK_GRID(grid), entry,    0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button,   0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check,    1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), scale,    0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), progress, 0, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label,    0, 4, 2, 1);

    // 시그널 연결
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), &w);
    g_signal_connect(scale, "value-changed", G_CALLBACK(on_scale_changed), &w);

    // 타이머 등록 (0.05초마다 progress 자동 업데이트)
    g_timeout_add(50, on_timer_tick, &w);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
